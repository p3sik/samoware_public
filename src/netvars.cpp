
#include "samoware/netvars.h"
#include "samoware/interfaces.h"
		 
#include "samoware/sdk/chlclient.h"

namespace netvars {
	std::unordered_map<std::string, int> netvars;

	namespace detail {
		void loadFromTable(RecvTable* tbl, char* tableName, int offset = 0) {
			for (int i = 0; i < tbl->m_nProps; i++) {
				RecvProp& prop = tbl->m_pProps[i];

				RecvTable* child = prop.m_pDataTable;
				if (child && child->m_nProps > 0) {
					loadFromTable(child, tableName, prop.m_Offset + offset);
				}

				std::string key(tableName + std::string("->") + prop.m_pVarName);
				netvars[key] = prop.m_Offset + offset;
			}
		}
	}

	void init() {
		ClientClass* clientClass = interfaces::client->GetAllClasses();

		while (clientClass) {
			RecvTable* tbl = clientClass->m_pRecvTable;
			detail::loadFromTable(tbl, tbl->m_pNetTableName);
			clientClass = clientClass->m_pNext;
		}
	}
}
