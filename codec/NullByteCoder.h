#ifndef ECO_NULL_BYTE_CODER_H
#define ECO_NULL_BYTE_CODER_H
/*******************************************************************************
【功能】
    CTP编码器是一个压缩算法，用于压缩字节流，可用于网络传输等。

【算法】
    算法简介：CTP压缩算法是CTP公司实现Ftdc协议、证券Level2行情协议的压缩算法。
	算法原理：CTP压缩算法是一个简单的压缩算法，由于在数据对象转化为字节流进行
	          传输时，可能存在大量NULL（ASCII=0）字符，此算法就是对连续的NULL
			  字符进行压缩。

	算法实现：将原文中"1-N(N>1)"个NULL字符表示为1个特殊字符，解压时再将特殊字符
	          还原。
			  1.特殊字符指集合(0xe0,0xef]中的字符，表示规则为特殊字符X
			    即0xe0<X<=0xef)表示N=(X-0xe0)个NULL字符，
			    如'0x00,0x00,0x00,0x00'压缩后为'0xe4'(=0xe0+4)。
			  2.原文中的特殊字符通过转义字符0xe0表示，即原文中的1个特殊字符，
			    压缩后需要用2个字符表示，如'0xe1'压缩后为'0xe0,0xe1'。
			  3.其他字符不处理。

	算法案例：压缩前：0x02 0x00 0x00 0x08 0x04 0xec 0x00 0x00 0x00 0x00
			  压缩后：0x02 [0xe2] 0x08 0x04 [0xe0 0xec] [0xe4]
			  括号中的内容为压缩处理的内容。

*******************************************************************************/
#include <eco/Project.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class NullByteCoder
{
public:
	// 压缩
	inline static void Encode(
		OUT std::string& cmp_data,
		IN  const std::string& raw_data)
	{
		cmp_data.clear();
		cmp_data.reserve(raw_data.size());

		// 前8个字节不用压缩
		const size_t cmp_pos = 8;
		cmp_data.insert(cmp_data.end(), raw_data.begin(), raw_data.begin()+cmp_pos);
		unsigned char max_cmp_null_num = 0xef - 0xe0;

		// 压缩算法
		// 压缩前：0x02 0x00 0x00 0x08 0x04 0xec 0x00 0x00 0x00 0x00
		// 压缩后：0x02 0xe2 0x08 0x04 0xe0 0xec 0xe4 
		for (size_t ri=cmp_pos; ri<raw_data.size(); ++ri)
		{
			unsigned char ri_char = raw_data[ri];
			// 1.原文为1-N个NULL字符。（需压缩的字符）
			if (ri_char == 0x00)
			{
				size_t null_count = 0;
				size_t ni = ri;
				while ((ni < raw_data.size()) && 
					((unsigned char)(raw_data[ni]) == 0x00))
				{
					++ni, ++null_count;
				}
				ri += null_count - 1;	// 遍历下个位置

				// 转换为特殊字符
				while (null_count >= max_cmp_null_num)
				{
					cmp_data.push_back(char(0xef));
					null_count -= max_cmp_null_num;
				}
				if (null_count > 0)
				{
					cmp_data.push_back(0xe0+null_count);
				}
			}
			// 2.原文为“0xe0-0xef”的特殊字符。（需转义的字符）
			else if (0xe0 <= ri_char && ri_char <= 0xef)
			{
				cmp_data.push_back(char(0xe0));
				cmp_data.push_back(raw_data[ri]);
			}
			// 3.原文为正常字符。（不需压缩的字符）
			else
			{
				cmp_data.push_back(raw_data[ri]);
			}
		}
	}

	// 解压
	inline static void Decode(
		OUT std::string& raw_data,
		IN  const std::string& cmp_data)
	{
		// 解压前：0x02 0xe2 0x08 0x04 0xe0 0xec 0xe4 
		// 解压后：0x02 0x00 0x00 0x08 0x04 0xec 0x00 0x00 0x00 0x00
		raw_data.clear();
		raw_data.reserve(cmp_data.size() * 3);

		// 前八个字节不用解压
		if (cmp_data.size() >= 8)
		{
			raw_data.insert(raw_data.end(), 
				cmp_data.begin(), cmp_data.begin()+8);
		}

		// 解压算法
		for (size_t ci=8; ci<cmp_data.size(); ++ci)
		{
			// 1.原文为正常字符。（不需压缩的字符）
			unsigned char ci_char(cmp_data[ci]);
			if (ci_char < 0xe0 || ci_char > 0xef)
			{
				raw_data.push_back(cmp_data[ci]);
			}
			// 2.原文为1-N个NULL字符。（需压缩的字符）
			else if (ci_char > 0xe0 && ci_char <= 0xef)
			{
				raw_data.insert(raw_data.end(), ci_char-0xe0, 0);
			}
			// 3.原文为“0xe0-0xef”的特殊字符。（需转义的字符）
			else if (
				(ci_char == 0xe0 && ci+1 < cmp_data.size()) &&
				((unsigned char)(cmp_data[ci+1]) >= 0xe0) &&
				((unsigned char)(cmp_data[ci+1]) <= 0xef))
			{
				// 原文特殊字符：1个字符表示为2个字符。
				raw_data.push_back(cmp_data[ci+1]);
				++ci;
			}
			else{assert(false);}
		}
	}
};


}// ns::eco
////////////////////////////////////////////////////////////////////////////////
#endif