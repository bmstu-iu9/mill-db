#include "Environment.h"

Environment* Environment::m_pInstance = nullptr;

Environment* Environment::instance() {
	if (!m_pInstance)
		m_pInstance = new Environment;

	return m_pInstance;
}

void Environment::add_table(Table* table) {
	this->tables.insert({table->get_name(), table});
}