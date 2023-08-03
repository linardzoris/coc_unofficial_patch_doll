#pragma once


class CBlender_blur : public IBlender
{
public:
	virtual LPCSTR getComment() { return "Blur generation"; }
	virtual BOOL canBeDetailed() { return FALSE; }
	virtual BOOL canBeLMAPped() { return FALSE; }

	virtual void Compile(CBlender_Compile& C);

	CBlender_blur();
	virtual ~CBlender_blur();
};

// Нихуя себе реверс жопой. Как тебе такое, Илон Маск?

class CBlender_nightvision : public IBlender
{
public:
    virtual LPCSTR getComment() { return "nightvision"; }
    virtual BOOL canBeDetailed() { return FALSE; }
    virtual BOOL canBeLMAPped() { return FALSE; }

    virtual void Compile(CBlender_Compile& C);

    CBlender_nightvision();
    virtual ~CBlender_nightvision();
};

class CBlender_pp_bloom : public IBlender
{
public:
    virtual LPCSTR getComment() { return "Nice bloom bro!"; }
    virtual BOOL canBeDetailed() { return FALSE; }
    virtual BOOL canBeLMAPped() { return FALSE; }

    virtual void Compile(CBlender_Compile& C);

    CBlender_pp_bloom();
    virtual ~CBlender_pp_bloom();
};
