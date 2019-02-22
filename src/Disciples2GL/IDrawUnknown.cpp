#include "stdafx.h"
#include "IDrawUnknown.h"

HRESULT __stdcall IDrawUnknown::QueryInterface(REFIID, LPVOID*) { return DD_OK; }

ULONG __stdcall IDrawUnknown::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall IDrawUnknown::Release()
{
	ULONG count = --this->refCount;
	if (!count)
		delete this;

	return count;
}

VOID IDrawDestruct(IDrawUnknown* item)
{
	IDrawUnknown* entry = *item->list;

	if (entry == item)
		*item->list = NULL;
	else while (entry)
	{
		if (entry->last == item)
		{
			entry->last = item->last;
			break;
		}

		entry = entry->last;
	}
}
