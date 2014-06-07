
#ifndef _STRINGJOIN_
#define _STRINGJOIN_

template<unsigned int LENGTH>
class StringJoin
{
public:
	StringJoin() 
	{
		memset(m_szData, 0, sizeof(m_szData));
	}
	~StringJoin() 
	{
	}

	StringJoin & operator + (short n)
	{
		sprintf(m_szData, "%s%d", m_szData, n);
		return *this;
	}
	StringJoin & operator + (unsigned short n)
	{
		sprintf(m_szData, "%s%d", m_szData, n);
		return *this;
	}
	StringJoin & operator + (int n)
	{
		sprintf(m_szData, "%s%d", m_szData, n);
		return *this;
	}
	StringJoin & operator + (unsigned int n)
	{
		sprintf(m_szData, "%s%d", m_szData, n);
		return *this;
	}
	StringJoin & operator + (long n)
	{
		sprintf(m_szData, "%s%ld", m_szData, n);
		return *this;
	}
	StringJoin & operator + (unsigned long n)
	{
		sprintf(m_szData, "%s%ld", m_szData, n);
		return *this;
	}
	StringJoin & operator + (float n)
	{
		sprintf(m_szData, "%s%f", m_szData, n);
		return *this;
	}
	StringJoin & operator + (double n)
	{
		sprintf(m_szData, "%s%lf", m_szData, n);
		return *this;
	}
	StringJoin & operator + (const char *p)
	{
		sprintf(m_szData, "%s%s", m_szData, p);
		return *this;
	}
	StringJoin & operator + (bool b)
	{
		if (b)
		{
			sprintf(m_szData, "%strue", m_szData);
		}
		else
		{
			sprintf(m_szData, "%sfalse", m_szData);
		}

		return *this;
	}

	const char * GetData() const { return m_szData; }

private:
	char m_szData[LENGTH];

};


#endif