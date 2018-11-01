#pragma once


class MessageInfo
{
	static const size_t m_buff_len = 0x3fff;
public:

	static void InitMessageInfo() {return }
	
	static MessageInfo* Get() {return new MessageInfo();}

	static void Free(MessageInfo* msg) {delete msg;}

	static size_t GetMaxSize() {return m_buff_len;}

	size_t GetSize() const 
	{
		size_t s = *((uint16*)m_buf);
		return s;
	}
	
	bool IncreaseSentCount()
	{
		static const int max_try_send_count = 5;
		char sentCount = m_buf[m_buff_len];
		if (sentCount < max_try_send_count) 
		{
			++m_buf[m_buff_len];
			return true;
		}
		return false;
	}
	
	size_t SequenceNumber() const
	{
		size_t sn = *((uint32*)(m_buf + 2));
		return sn;
	}
	
	
	
private:
	MessageInfo() 
	{
		memset(this, 0, sizeof(MessageInfo));
	}

	MessageInfo(const MessageInfo&);

private:
	char m_buf[m_buff_len + 1];
	static MessageInfo* m_messageInfos;
};
