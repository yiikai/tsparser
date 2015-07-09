#include "TsFileParser.h"
const int PACKETSIZE=188;

const int PAT=0;
const int PMT = 1;
const int PES = 2;

//PES StreamID
const int program_stream_map = 0xBC;
const int private_stream_2 = 0xBF;
const int padding_stream = 0xBE;
const int ECM_stream = 0xF0;
const int EMM_stream = 0xF1;
const int program_stream_directory = 0xFF;
const int DSMCC_stream = 0xF2;
const int type_E_stream = 0xF8;




TsFileParser::TsFileParser() :BaseParser()
, m_pat(NULL)
{

}

TsFileParser::~TsFileParser()
{

}

int TsFileParser::Parse()
{
	int ret;
	while (1)
	{
		std::shared_ptr<std::vector<char>> packet;
		int offset = 0;
		ret = GetTsPacket(packet);
		if (ret == PARSER_FAIL)
		{
			std::cout << "get ts packet error" << std::endl;
			break;
		}
			
		PAKHEAD_ST packethead;
		memset(&packethead, 0, sizeof(PAKHEAD_ST));
		checkPacket(packet, packethead);
		offset += 4; 
		
		if (packethead.isPSI)
			offset++;   //if The TS packet playload is psi info and need skip one byte of pointer_field
		if (packethead.adaptation_field_control == 2 || packethead.adaptation_field_control == 3){
			//have adaption_filed and need to parser
			if (packethead.adaptation_func_cb)
			{
				packethead.adaptation_func_cb(packet, offset, this, &packethead);
				offset = 0;
			}
			else
			{
				continue;
			}
		}
		if (packethead.func_cb)
			packethead.func_cb(packet,offset,this,&packethead);
	}
	return ret;
}

int TsFileParser::GetTsPacket(std::shared_ptr<std::vector<char>>& packet)
{
	packet = m_fileoperator.readBytes(PACKETSIZE);
	if (packet == nullptr)
		return PARSER_FAIL;
	return PARSER_OK;
}


c_int64 TsFileParser::getPTS(std::shared_ptr<std::vector<char>>& packet,int offset)
{
	return get_pts_or_dts(packet, offset);
}

c_int64 TsFileParser::get_pts_or_dts(std::shared_ptr<std::vector<char>>& buf, int offset)
{	//before get pts and dts must skip 4bit 0010
	unsigned char  ptsflag;
	unsigned char peshdr_datalen;
	int index = offset;

	c_int64 pts_or_dts;
	unsigned short pts29_15, pts14_0;
	c_int64 pts32_30 = ((*buf)[index] & 0x0e) >> 1;
	index++;
	pts29_15 = MKWORD((*buf)[index], (*buf)[index + 1] & 0xfe) >> 1;
	index += 2;
	pts14_0 = MKWORD((*buf)[index], (*buf)[index + 1] & 0xfe) >> 1;
	pts_or_dts = (pts32_30 << 30) | (pts29_15 << 15) | pts14_0;
	pts_or_dts = pts_or_dts;
	return pts_or_dts;
}

c_int64 TsFileParser::getDTS(std::shared_ptr<std::vector<char>>& packet, int offset)
{
	return get_pts_or_dts(packet,offset);
}

int section_cb::PAT_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head)
{
	if ((*packet)[offset] != 0x00)
	{
		return PARSER_FAIL;
	}
	offset++;
	std::shared_ptr<PAT_ST> pat_st(new PAT_ST());
	pat_st->table_id = 0x00;
	unsigned char data = (*packet)[offset] & 0x80;
	pat_st->section_syntax_indicator = data >> 7;
	char length[2];
	length[1] = (*packet)[offset] & 0x0f;
	offset++;
	length[0] = (*packet)[offset];
	memcpy(&(pat_st->section_length),length,2);
	offset++;
	char ID[2];
	ID[1] = (*packet)[offset];
	offset++;
	ID[0] = (*packet)[offset];
	memcpy(&(pat_st->transport_stream_id), ID, 2);
	offset++;
	data = (*packet)[offset] & 0x3E;
	pat_st->version_number = data >> 1;
	data = (*packet)[offset] & 0x01;
	pat_st->current_next_indicator = data;
	offset++;
	pat_st->section_number = (*packet)[offset];
	offset++;
	pat_st->last_section_number = (*packet)[offset];
	int programnum = pat_st->section_length - 5 - 4;   //从剩下的pat表数据中获取program info ,最后减掉4个字节是CRC
	offset++;
	for (int i = 0; i < programnum;i++)
	{
		int program_number = 0,program_id = 0;
		char program[2];
		program[1] = (*packet)[offset + i];
		i++;
		program[0] = (*packet)[offset + i];
		memcpy(&program_number, program, 2);
		i++;
		char data = (*packet)[offset + i] & 0x1F;
		program[1] = data;
		i++;
		program[0] = (*packet)[offset + i];
		memcpy(&program_id,program,2);
		pat_st->program_table_list.push_back(std::make_pair(program_number, program_id));
	}
	goodfriend->m_pat = pat_st;
	return PARSER_OK;
}



int section_cb::PES_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head)
{
	if ((*packet)[offset] != 0x00 || (*packet)[++offset] != 0x00 || (*packet)[++offset] != 0x01)
	{
		std::cout << "not contain PES packet data" << std::endl;
		return PARSER_FAIL;
	}
	std::shared_ptr<PES_ST> pes_st(new PES_ST);
	pes_st->streamselectid = head->PID;
	offset++;
	pes_st->streamid = (*packet)[offset];
	offset++;
	char data[2];
	data[1] = (*packet)[offset];
	offset++;
	data[0] = (*packet)[offset];
	memcpy(&(pes_st->pes_packet_length), data, 2);
	int pesEnd = 0;
	if (pes_st->pes_packet_length > 0)
		pesEnd = offset + pes_st->pes_packet_length;
	else
		pesEnd = PACKETSIZE;
	offset++;
	int playloadstart = 0;
	if (pes_st->streamid != program_stream_map &&
		pes_st->streamid != private_stream_2 &&
		pes_st->streamid != padding_stream &&
		pes_st->streamid != ECM_stream &&
		pes_st->streamid != EMM_stream &&
		pes_st->streamid != program_stream_directory &&
		pes_st->streamid != DSMCC_stream &&
		pes_st->streamid != type_E_stream)
	{
		if ((((*packet)[offset] & 0xC0) >> 6) != 2)
		{
			std::cout << "format is wrong and the frist is need be 10" << std::endl;
		}
		pes_st->PES_scrambling_control = ((*packet)[offset] & 0x30) >> 4;
		pes_st->PES_priority = ((*packet)[offset] & 0x08) >> 3;
		pes_st->data_alignment_indicator = ((*packet)[offset] & 0x04) >> 2;
		pes_st->copyright = ((*packet)[offset] & 0x02) >> 1;
		pes_st->original_or_copy = (*packet)[offset] & 0x01;
		offset++;
		pes_st->PTS_DTS_flags = ((*packet)[offset] & 0xC0) >> 6;
		if (pes_st->PTS_DTS_flags == 1)
		{
			std::cout << "pts and dts flag value is forbidden." << std::endl;
			return PARSER_FAIL;
		}
		pes_st->ESCR_flag = ((*packet)[offset] & 0x20) >> 5;
		pes_st->ES_rate_flag = ((*packet)[offset] & 0x10) >> 4;
		pes_st->DSM_trick_mode_flag = ((*packet)[offset] & 0x08) >> 3;
		pes_st->additional_copy_info_flag = ((*packet)[offset] & 0x04) >> 2;
		pes_st->PES_CRC_flag = ((*packet)[offset] & 0x02) >> 1;
		pes_st->PES_extension_flag = ((*packet)[offset] & 0x01);
		offset++;
		pes_st->PES_header_data_length = (*packet)[offset];
		pes_st->playloadsize = pesEnd - offset - pes_st->PES_header_data_length;
		playloadstart = offset + pes_st->PES_header_data_length;
		offset++;
		switch (pes_st->PTS_DTS_flags)
		{
		case 2:
		{
			pes_st->pts = goodfriend->getPTS(packet, offset);
			offset += 5;
		}break;
		case 3:
		{
			pes_st->pts = goodfriend->getPTS(packet, offset);
			offset += 5;
			pes_st->dts = goodfriend->getDTS(packet, offset);
			offset += 5;
		}
		default:
			break;
		}
		if (pes_st->DSM_trick_mode_flag == 1)
		{

		}
		if (pes_st->additional_copy_info_flag == 1)
		{

		}
		if (pes_st->PES_CRC_flag == 1)
		{

		}
		if (pes_st->PES_extension_flag == 1)
		{

		}
	}
	std::shared_ptr<std::vector<char>> playload(new std::vector<char>((*packet).begin() + playloadstart+1,(*packet).end()));
	pes_st->playloadbuf = playload;

	goodfriend->putPESStreamData(pes_st);
	return PARSER_OK;
}

int TsFileParser::putPESStreamData(std::shared_ptr<PES_ST> pesdata)
{
	std::list<streaminfo_st>::iterator itr = m_currentselectpmt->streamlist.begin();
	for (; itr != m_currentselectpmt->streamlist.end(); itr++)
	{
		if (pesdata->streamselectid == itr->element_pid)
		{
			itr->streamplayloadlist.push_back(pesdata);
		}
	}
	return PARSER_OK;
}

int section_cb::PES_Packet_Compose(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head)
{
	//进行pes的包拼装，直接获取packet头4个字节之后的数据
	int ret = goodfriend->packetPESStreamData(head, packet, offset);
	return ret;
}

int TsFileParser::packetPESStreamData(PAKHEAD_ST *head, std::shared_ptr<std::vector<char>>& packet, int offset)
{
	std::list<streaminfo_st>::iterator itr = m_currentselectpmt->streamlist.begin();
	for (; itr != m_currentselectpmt->streamlist.end(); itr++)
	{
		if (head->PID == itr->element_pid)
		{
			std::shared_ptr<std::vector<char>> packetdata(new std::vector<char>((*packet).begin() + offset,(*packet).end()));
			if (itr->streamplayloadlist.empty())
			{
				//std::cout << "the pes paceket not include any stream in the Program streamlist" << std::endl;
				return PARSER_FAIL;
			}
			(*(--itr->streamplayloadlist.end()))->playloadbuf->insert((*(--itr->streamplayloadlist.end()))->playloadbuf->end(), packetdata->begin(), packetdata->end());
			break;
		}
	}
	return PARSER_OK;
}

int section_cb::PMT_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head)
{
	if ((*packet)[offset] != 0x02)
	{
		std::cout << "not PMT table " << std::endl;
		return PARSER_FAIL;
	}
	std::shared_ptr<PMT_ST> pmt_st(new PMT_ST());
	pmt_st->table_id = (*packet)[offset]; 
	offset++;
	pmt_st->section_syntax_length = ((*packet)[offset] & 0x80) >> 7;
	if (pmt_st->section_syntax_length != 1)
	{
		std::cout << "not PMT section_syntax_indicator value is " << pmt_st->section_syntax_length << std::endl;
		return PARSER_FAIL;
	}
	char data[2];
	data[1] = (*packet)[offset] & 0x0f;
	offset++;
	data[0] = (*packet)[offset];
	memcpy(&(pmt_st->section_length), data, 2);
	offset++;
	data[1] = (*packet)[offset];
	offset++;
	data[0] = (*packet)[offset];
	memcpy(&(pmt_st->program_number),data,2);
	offset++;
	pmt_st->version_number = ((*packet)[offset] & 0x1f >> 1);
	pmt_st->current_next_indicator = (*packet)[offset] & 0x01;
	offset++;
	pmt_st->section_number = (*packet)[offset];
	offset++;
	pmt_st->last_section_number = (*packet)[offset];
	offset++;
	data[1] = (*packet)[offset] & 0x1f;
	offset++;
	data[0] = (*packet)[offset];
	memcpy(&(pmt_st->pcr_id), data, 2);
	offset++;
	data[1] = (*packet)[offset] & 0x0f;
	offset++;
	data[0] = (*packet)[offset];
	memcpy(&(pmt_st->program_info_length),data,2);
	offset += pmt_st->program_info_length;  //跳过program detailinfo 数据
	offset++;
	int totalstreamnum = pmt_st->section_length + 8 - offset - 4;  //8:pmt section_length之前的数据字节数  4：最后的crc校验位值
	for (int i = 0; i < totalstreamnum; i++)
	{
		streaminfo_st streaminfo;
		streaminfo.stream_type = (*packet)[offset + i];
		i++;
		char pid[2];
		pid[1] = (*packet)[offset + i] & 0x1f;
		i++;
		pid[0] = (*packet)[offset + i];
		memcpy(&(streaminfo.element_pid), pid, 2);
		i++;
		int eslen = 0;
		char esinfolen[2];
		esinfolen[1] = (*packet)[offset + i] & 0x0f;
		i++;
		esinfolen[0] = (*packet)[offset + i];
		memcpy(&eslen, esinfolen, 2);
		i += eslen;   //目前对于解析的descriptinfo 不做处理先跳过，以后再详细了解 
		pmt_st->streamlist.push_back(streaminfo);
		//goodfriend->m_pmtMap.insert(std::make_pair(streaminfo.element_pid, pmt_st));
	}
	goodfriend->m_pmtMap.insert(std::make_pair(pmt_st->program_number, pmt_st));
	return PARSER_OK;
}

int section_cb::ADAPTION_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head)
{
	std::shared_ptr<ADAPTION_ST> adapt_st(new ADAPTION_ST());
	adapt_st->adaptation_field_length = (*packet)[offset];
	if (adapt_st->adaptation_field_length > 0)
	{
		std::vector<char> *remaindata = new std::vector<char>(packet->begin() + offset + adapt_st->adaptation_field_length + 1, packet->end());
		offset++;
		adapt_st->discontinuity_indicator = ((*packet)[offset] & 0x80) >> 7;
		adapt_st->random_access_indicator = ((*packet)[offset] & 0x40) >> 6;
		adapt_st->elementary_stream_priority_indicator = ((*packet)[offset] & 0x20) >> 5;
		adapt_st->PCR_flag = ((*packet)[offset] & 0x10) >> 4;
		adapt_st->OPCR_flag = ((*packet)[offset] & 0x08) >> 3;
		adapt_st->splicing_point_flag = ((*packet)[offset] & 0x04) >> 2;
		adapt_st->transport_private_data_flag = ((*packet)[offset] & 0x02) >> 1;
		adapt_st->adaptation_field_extension_flag = ((*packet)[offset] & 0x01);
		if (adapt_st->PCR_flag == 1)
		{

		}
		if (adapt_st->OPCR_flag == 1)
		{

		}
		if (adapt_st->splicing_point_flag == 1)
		{

		}
		if (adapt_st->transport_private_data_flag == 1)
		{

		}
		if (adapt_st->adaptation_field_extension_flag == 1)
		{

		}
		packet.reset(remaindata);
	}
	return PARSER_OK;
}

int TsFileParser::clearTSAllProgram()
{
	if (m_pat != NULL)
		m_pat = NULL;
	if (!m_pmtMap.empty())
		m_pmtMap.clear();
	if (m_currentselectpmt != NULL)
		m_currentselectpmt = NULL;
	return PARSER_OK;
}

int TsFileParser::checkPacket(const std::shared_ptr<std::vector<char>>& packet, PAKHEAD_ST& head)
{
	if ((*packet)[0] != 0x47)
	{
		return PARSER_FAIL;
	}
	char data = (*packet)[1] & 0xE0;
	head.transport_error_indicator = data >> 7;
	data = (*packet)[1] & 0x60;
	head.payload_unit_start_indicator = data >> 6;
	data = (*packet)[1] & 0x20;
	head.transport_priority = data >> 5;
	if ((((*packet)[1] & 0x1F )== 0x00) && ((*packet)[2] == 0x00))  //TS文件第一个packet是PAT
	{
		std::cout << "The packet playload is PAT table" << std::endl;
		clearTSAllProgram();
		head.PID = PAT;
		head.func_cb = section_cb::PAT_Handler;
		head.isPSI = true;
	}
	else //不是PAT表就要看这个packet是PMT表还是其他的
	{
		//获取当前packet的tableID，确定了tableID 进行后续的操作 
		int id = 0;
		char pmt[2];
		pmt[1] = (*packet)[1] & 0x1F;
		pmt[0] = (*packet)[2];
		memcpy(&id,pmt,2);
		if (m_pat != NULL && !m_pat->program_table_list.empty())
		{
			std::list<std::pair<int, int>>::iterator itr = m_pat->program_table_list.begin();
			for (; itr != m_pat->program_table_list.end(); itr++)
			{
				if (itr->second == id)
				{
					std::cout << "The packet playload is PMT table" << std::endl;
					head.PID = PMT;
					head.func_cb = section_cb::PMT_Handler;
					head.isPSI = true;
					goto end;
				}
			}
		}
		//目前ts文件中只有一个节目，先写死对这一个文件里的流做判断选择
		if (!m_pmtMap.empty())
		{


			m_currentselectpmt = m_pmtMap.begin()->second;
			std::list<streaminfo_st>::iterator streamitr = m_pmtMap.begin()->second->streamlist.begin();
			for (; streamitr != m_pmtMap.begin()->second->streamlist.end(); streamitr++)
			{
				if ((*streamitr).element_pid == id)
				{
					//std::cout << "The packet playload is Stream id" << std::endl;
					if (head.payload_unit_start_indicator == 0x01)
					{
						head.PID = id;
						head.func_cb = section_cb::PES_Handler;
						head.isPSI = false;
					}
					else if (head.payload_unit_start_indicator == 0x00)  //是PES的节目流数据并且payload_unit_start_indicator为0，需要进行pes的包的拼包获取完整的一帧数据
					{
						head.PID = id;
						head.func_cb = section_cb::PES_Packet_Compose;
						head.isPSI = false;
					}
					else
					{
						std::cout << "not occur the case" << std::endl;
					}
					goto end;
				}
			}
			/*if (m_pmtMap.find(id) != m_pmtMap.end())
			{
			std::cout << "The packet playload is Stream id" << std::endl;
			if (head.payload_unit_start_indicator == 0x01)
			{
			head.PID = id;
			head.func_cb = section_cb::PES_Handler;
			head.isPSI = false;
			}
			goto end;
			}*/
		}
		else
		{
			std::cout << "current stream not included in any PMT table" << std::endl;
			return PARSER_FAIL;
		}
		//发现的pid不是PMT表的就应该是stream的pid.
	}
end:
	data = (*packet)[3] & 0xC0;
	head.transport_scrambling_control = data >> 6;
	data = (*packet)[3] & 0x30;
	head.adaptation_field_control = data >> 4;
	if (head.adaptation_field_control == 3 || head.adaptation_field_control == 2)
	{
		head.adaptation_func_cb = section_cb::ADAPTION_Handler;
	}
	else
	{
		head.adaptation_func_cb = NULL;
	}
	data = (*packet)[3] & 0x0ff;
	head.continuity_counter = data;
	return PARSER_OK;
}