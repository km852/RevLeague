#pragma once

struct SBlock
{
	SBlock(unsigned int l = 0, unsigned int r = 0) : m_uil(l), m_uir(r) {}
	SBlock(const SBlock& roBlock) : m_uil(roBlock.m_uil), m_uir(roBlock.m_uir) {}
	SBlock& operator^=(SBlock& b) { m_uil ^= b.m_uil; m_uir ^= b.m_uir; return *this; }
	unsigned int m_uil, m_uir;
};

class CBlowFish
{
public:
	CBlowFish(unsigned char* ucKey, size_t n);

	void Encrypt(unsigned char* buf, size_t n);
	void Decrypt(unsigned char* buf, size_t n);

private:
	unsigned int F(unsigned int ui) const;
	void Encrypt(SBlock&);
	void Decrypt(SBlock&);

private:
	unsigned int m_auiP[18];
	unsigned int m_auiS[4][256];
	static const unsigned int scm_auiInitP[18];
	static const unsigned int scm_auiInitS[4][256];
};
