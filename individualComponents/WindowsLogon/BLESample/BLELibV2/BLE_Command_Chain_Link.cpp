#include "BLE_Command_Chain_Link.h"
#include <string.h>

IBLE_COMMAND_CHAIN_LINK::IBLE_COMMAND_CHAIN_LINK()
{
	nextLink = nullptr;
}

IBLE_COMMAND_CHAIN_LINK::~IBLE_COMMAND_CHAIN_LINK()
{
	if (nextLink != nullptr)
	{
		delete nextLink;
		nextLink = nullptr;
	}
}

const IBLE_COMMAND_CHAIN_LINK* IBLE_COMMAND_CHAIN_LINK::getNextLink() const
{
	return nextLink;
}

void IBLE_COMMAND_CHAIN_LINK::appendLink(IBLE_COMMAND_CHAIN_LINK* link)
{
	IBLE_COMMAND_CHAIN_LINK* pLink = this;

	while (true)
	{
		if (link == pLink)
		{
			break; //link already exists in our list
		}

		if (pLink->nextLink != nullptr)
		{
			pLink = pLink->nextLink;
		}
		else
		{
			pLink->nextLink = link;
			if (link->nextLink != nullptr)
			{
				IBLE_COMMAND_CHAIN_LINK* tempLink = link->nextLink;
				link->nextLink = nullptr;
				this->appendLink(tempLink);
			}
			break;
		}
	}
}

BLE_COMMAND_CHAIN_LINK_FULL::BLE_COMMAND_CHAIN_LINK_FULL()
{
	this->ble.default = nullptr;
	memset(&evt, 0, sizeof(evt));
	memset(&rsp, 0, sizeof(rsp));
}

bool BLE_COMMAND_CHAIN_LINK_FULL::isLinkModular() const
{
	return false;
}

BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD::BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD()
{
	this->default = nullptr;
	this->evt = nullptr;
	this->rsp = nullptr;
}

bool BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD::isLinkModular() const
{
	return true;
}
