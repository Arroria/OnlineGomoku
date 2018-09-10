#pragma once

template <typename class_t>
class Singleton
{
public:
	static class_t& GetSingleton()
	{
		if (!m_inst)
			m_inst = new class_t;
		return *m_inst;
	}
	static void ReleaseSingleton()
	{
		if (m_inst)
		{
			delete m_inst;
			m_inst = nullptr;
		}
	}

private:
	static class_t* m_inst;
};

template <typename class_t>
class_t* Singleton<class_t>::m_inst = nullptr;


#define SntInst(class_t) (Singleton<class_t>::GetSingleton())
#define SntRelease(class_t) (Singleton<class_t>::ReleaseSingleton())
