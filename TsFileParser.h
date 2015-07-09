#ifndef TS_FILE_PARSER_H
#define TS_FILE_PARSER_H
#include "BaseParser.h"
#include <list>
#include <map>
#include <utility>

class TsFileParser;
struct packet_st;
typedef struct  packet_st PAKHEAD_ST;
typedef int(*type_func)(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head);

typedef struct packet_st
{
	unsigned char transport_error_indicator : 1;
	unsigned char payload_unit_start_indicator : 1;
	unsigned char transport_priority : 1;
	int PID : 13;
	unsigned char transport_scrambling_control : 2;
	unsigned char adaptation_field_control : 2;
	unsigned char continuity_counter : 4;
	type_func func_cb;
	type_func adaptation_func_cb;
	bool isPSI;
}PAKHEAD_ST;

//PAT table
typedef struct _pat_st
{
	_pat_st() :section_length(0)
		, transport_stream_id(0)
	{

	}
	unsigned char table_id;
	unsigned char section_syntax_indicator : 1;
	int section_length;
	int transport_stream_id;
	unsigned char version_number : 5;
	unsigned char current_next_indicator : 1;
	unsigned char section_number;
	unsigned char last_section_number;
	std::list<std::pair<int, int>> program_table_list;
}PAT_ST;

struct _pes_st;
typedef struct _pes_st PES_ST;
typedef struct _stream_info_st
{
	_stream_info_st() :element_pid(0)
	{

	}

	unsigned char stream_type;
	int element_pid;
	std::list<std::shared_ptr<PES_ST>> streamplayloadlist;

}streaminfo_st;

typedef struct _pmt_st
{
	_pmt_st() :section_length(0)
		, program_number(0)
		, pcr_id(0)
		, program_info_length(0)
	{

	}
	unsigned char table_id;
	unsigned char section_syntax_length : 1;
	int section_length;
	int program_number;
	unsigned char version_number : 5;
	unsigned char current_next_indicator : 1;
	unsigned char section_number;
	unsigned char last_section_number;
	int pcr_id;
	int program_info_length;
	std::list<streaminfo_st> streamlist;
}PMT_ST;

typedef struct  _adaption_st
{
	_adaption_st()
	{

	}
	unsigned char adaptation_field_length;
	unsigned char discontinuity_indicator : 1;
	unsigned char random_access_indicator : 1;
	unsigned char elementary_stream_priority_indicator : 1;
	unsigned char PCR_flag : 1;
	unsigned char OPCR_flag : 1;
	unsigned char splicing_point_flag : 1;
	unsigned char transport_private_data_flag : 1;
	unsigned char adaptation_field_extension_flag : 1;

}ADAPTION_ST;

typedef struct _pes_st
{
	_pes_st() :pes_packet_length(0)
		, pts(PTS_NO_VALUE)
		, dts(DTS_NO_VALUE)
		, playloadsize(0)
		, streamselectid(0)
	{

	}
	unsigned char streamid;
	int pes_packet_length;
	unsigned char PES_scrambling_control : 2;
	unsigned char PES_priority : 1;
	unsigned char data_alignment_indicator : 1;
	unsigned char copyright : 1;
	unsigned char original_or_copy : 1;
	unsigned char PTS_DTS_flags : 2;
	unsigned char ESCR_flag : 1;
	unsigned char ES_rate_flag : 1;
	unsigned char DSM_trick_mode_flag : 1;
	unsigned char additional_copy_info_flag : 1;
	unsigned char PES_CRC_flag : 1;
	unsigned char PES_extension_flag : 1;
	unsigned char PES_header_data_length;
	std::shared_ptr<std::vector<char>> playloadbuf;
	int playloadsize;

	c_int64 pts;
	c_int64 dts;
	int streamselectid;   //这个pes包中的流数据是pmt表中的哪个
}PES_ST;

class section_cb
{
public:
	static int PAT_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head = NULL);
	static int PMT_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head = NULL);
	static int PES_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head = NULL);
	static int ADAPTION_Handler(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head = NULL);
	static int PES_Packet_Compose(std::shared_ptr<std::vector<char>>& packet, int offset, TsFileParser* goodfriend, PAKHEAD_ST *head = NULL);
};
 

class TsFileParser : public BaseParser
{
public:
	TsFileParser();
	~TsFileParser();
	int Parse() override;
	int clearTSAllProgram();

private:
	int GetTsPacket(std::shared_ptr<std::vector<char>>& packet);
	int checkPacket(const std::shared_ptr<std::vector<char>>& packet, PAKHEAD_ST& head);
	int GetPacketHead();
	c_int64 getPTS(std::shared_ptr<std::vector<char>>& packet, int offset) override;
	c_int64 getDTS(std::shared_ptr<std::vector<char>>& packet, int offset) override;
	c_int64 get_pts_or_dts(std::shared_ptr<std::vector<char>>& packet, int offset);
	int putPESStreamData(std::shared_ptr<PES_ST> pesdata);
	int packetPESStreamData(PAKHEAD_ST *head, std::shared_ptr<std::vector<char>>& packet, int offset);
	
private:
	std::shared_ptr<PAT_ST> m_pat;
	std::map<int,std::shared_ptr<PMT_ST>> m_pmtMap;  //一个TS文件中会有1个或多个PMT表，代表了有多少个节目在里面
	std::shared_ptr<PMT_ST> m_currentselectpmt;   //表示当前选择的节目
	friend class section_cb;
};

#endif