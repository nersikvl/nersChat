
class UDPServer: public ThreadPool
{
public:	
	UDPServer() : ThreadPool()
	{}

protected:
   virtual void RunInLoop() 
   {
	   if (!RecvMessage() &&
		   !SendMessage())
	   {
         std::this_thread::sleep_for(time::Duration::from_millis(500));
	   }
   }
	
private:
	bool RecvMessage()
	{
		MessageInfo* msg = MessageInfo::Get();
		if (!msg) return false;
		
		Errors err = m_socet.recvMessage(msg);
		if (ErrOK == err)
		{
			if (msg->IsAck())
			{
				m_ackedMsgs.insert(msg->SequenceNumber());
			}
			else
			{
				recvMessages.push(msg);
			}
			return true;
		}

		return false;
	}

	bool SendMessage()
	{
		MessageInfo* msg = sendMessages.TryPop();
		if (!msg) return false;
		
		while (m_ackedMessages.end() != m_ackedMessages.find(msg->SequenceNumber()))
		{
			m_ackedMessages.erase(msg->SequenceNumber());
			MessageInfo::Free(msg);
			msg = sendMessages.TryPop();
			if (!msg) return true;
		}

		Errors err = m_socet.sendMessage(msg);
		if (ErrOK != err || msg->IncreaseSentCount())
		{
			sendMessages.push(msg);
		}
		else
		{
			MessageInfo::Free(msg);
		}

		return true;
	}
	   
private:
	SocketInfo 		   m_socet;
	Queue<MessageInfo> m_recvMessages;
	Queue<MessageInfo> m_sendMessages;
	set<size_t>		   m_ackedMessages;
};