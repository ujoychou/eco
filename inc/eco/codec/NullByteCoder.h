#ifndef ECO_NULL_BYTE_CODER_H
#define ECO_NULL_BYTE_CODER_H
/*******************************************************************************
�����ܡ�
    CTP��������һ��ѹ���㷨������ѹ���ֽ��������������紫��ȡ�

���㷨��
    �㷨��飺CTPѹ���㷨��CTP��˾ʵ��FtdcЭ�顢֤ȯLevel2����Э���ѹ���㷨��
	�㷨ԭ����CTPѹ���㷨��һ���򵥵�ѹ���㷨�����������ݶ���ת��Ϊ�ֽ�������
	          ����ʱ�����ܴ��ڴ���NULL��ASCII=0���ַ������㷨���Ƕ�������NULL
			  �ַ�����ѹ����

	�㷨ʵ�֣���ԭ����"1-N(N>1)"��NULL�ַ���ʾΪ1�������ַ�����ѹʱ�ٽ������ַ�
	          ��ԭ��
			  1.�����ַ�ָ����(0xe0,0xef]�е��ַ�����ʾ����Ϊ�����ַ�X
			    ��0xe0<X<=0xef)��ʾN=(X-0xe0)��NULL�ַ���
			    ��'0x00,0x00,0x00,0x00'ѹ����Ϊ'0xe4'(=0xe0+4)��
			  2.ԭ���е������ַ�ͨ��ת���ַ�0xe0��ʾ����ԭ���е�1�������ַ���
			    ѹ������Ҫ��2���ַ���ʾ����'0xe1'ѹ����Ϊ'0xe0,0xe1'��
			  3.�����ַ���������

	�㷨������ѹ��ǰ��0x02 0x00 0x00 0x08 0x04 0xec 0x00 0x00 0x00 0x00
			  ѹ����0x02 [0xe2] 0x08 0x04 [0xe0 0xec] [0xe4]
			  �����е�����Ϊѹ�����������ݡ�

*******************************************************************************/
#include <eco/rx/RxExport.h>
#include <string>


ECO_NS_BEGIN(eco);


////////////////////////////////////////////////////////////////////////////////
class NullByteCoder
{
public:
	// ѹ��
	inline static void Encode(
		OUT std::string& cmp_data,
		IN  const std::string& raw_data)
	{
		cmp_data.clear();
		cmp_data.reserve(raw_data.size());

		// ǰ8���ֽڲ���ѹ��
		const size_t cmp_pos = 8;
		cmp_data.insert(cmp_data.end(), raw_data.begin(), raw_data.begin()+cmp_pos);
		unsigned char max_cmp_null_num = 0xef - 0xe0;

		// ѹ���㷨
		// ѹ��ǰ��0x02 0x00 0x00 0x08 0x04 0xec 0x00 0x00 0x00 0x00
		// ѹ����0x02 0xe2 0x08 0x04 0xe0 0xec 0xe4 
		for (size_t ri=cmp_pos; ri<raw_data.size(); ++ri)
		{
			unsigned char ri_char = raw_data[ri];
			// 1.ԭ��Ϊ1-N��NULL�ַ�������ѹ�����ַ���
			if (ri_char == 0x00)
			{
				size_t null_count = 0;
				size_t ni = ri;
				while ((ni < raw_data.size()) && 
					((unsigned char)(raw_data[ni]) == 0x00))
				{
					++ni, ++null_count;
				}
				ri += null_count - 1;	// �����¸�λ��

				// ת��Ϊ�����ַ�
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
			// 2.ԭ��Ϊ��0xe0-0xef���������ַ�������ת����ַ���
			else if (0xe0 <= ri_char && ri_char <= 0xef)
			{
				cmp_data.push_back(char(0xe0));
				cmp_data.push_back(raw_data[ri]);
			}
			// 3.ԭ��Ϊ�����ַ���������ѹ�����ַ���
			else
			{
				cmp_data.push_back(raw_data[ri]);
			}
		}
	}

	// ��ѹ
	inline static void Decode(
		OUT std::string& raw_data,
		IN  const std::string& cmp_data)
	{
		// ��ѹǰ��0x02 0xe2 0x08 0x04 0xe0 0xec 0xe4 
		// ��ѹ��0x02 0x00 0x00 0x08 0x04 0xec 0x00 0x00 0x00 0x00
		raw_data.clear();
		raw_data.reserve(cmp_data.size() * 3);

		// ǰ�˸��ֽڲ��ý�ѹ
		if (cmp_data.size() >= 8)
		{
			raw_data.insert(raw_data.end(), 
				cmp_data.begin(), cmp_data.begin()+8);
		}

		// ��ѹ�㷨
		for (size_t ci=8; ci<cmp_data.size(); ++ci)
		{
			// 1.ԭ��Ϊ�����ַ���������ѹ�����ַ���
			unsigned char ci_char(cmp_data[ci]);
			if (ci_char < 0xe0 || ci_char > 0xef)
			{
				raw_data.push_back(cmp_data[ci]);
			}
			// 2.ԭ��Ϊ1-N��NULL�ַ�������ѹ�����ַ���
			else if (ci_char > 0xe0 && ci_char <= 0xef)
			{
				raw_data.insert(raw_data.end(), ci_char-0xe0, 0);
			}
			// 3.ԭ��Ϊ��0xe0-0xef���������ַ�������ת����ַ���
			else if (
				(ci_char == 0xe0 && ci+1 < cmp_data.size()) &&
				((unsigned char)(cmp_data[ci+1]) >= 0xe0) &&
				((unsigned char)(cmp_data[ci+1]) <= 0xef))
			{
				// ԭ�������ַ���1���ַ���ʾΪ2���ַ���
				raw_data.push_back(cmp_data[ci+1]);
				++ci;
			}
			else{assert(false);}
		}
	}
};


ECO_NS_END(eco);
////////////////////////////////////////////////////////////////////////////////
#endif