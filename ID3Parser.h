#ifndef ID3_PARSER_H
#define ID3_PARSER_H
#include <memory>
#include <vector>
typedef struct
{
	int size;
	int pts;
}ID3_ST;

class ID3Parser
{
public:
	ID3Parser();
	~ID3Parser();
	bool parse(std::shared_ptr<std::vector<unsigned char>> data);
	bool checkID3(std::shared_ptr<std::vector<unsigned char>> data);
	ID3_ST GetId3(){ return m_id3; }
private:
	ID3_ST m_id3;
};

#endif