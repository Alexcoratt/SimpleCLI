#include "BaseOptionContainer.hpp"
#include "BackOption.hpp"
#include "CliException.hpp"
#include "HelpOption.hpp"
#include "IOption.hpp"
#include "OptionAlreadyExistsException.hpp"
#include "OptionBackException.hpp"
#include "OptionDoesNotExistException.hpp"
#include "OptionQuitException.hpp"
#include "QuitOption.hpp"
#include <stdexcept>

BaseOptionContainer::BaseOptionContainer(std::string const & name, std::string const & help, std::map<char, IOption *> const & options, bool noBackOption) : _name(name), _help(help) {
	IOption * helpOption = new HelpOption;
	addOption('h', helpOption);
	helpOption->setNoDelete(false);
	helpOption->setIsBaseOption(true);

	IOption * quitOption = new QuitOption;
	_options['q'] = quitOption;
	quitOption->setNoDelete(false);
	quitOption->setIsBaseOption(true);

	if (!noBackOption) {
		IOption * backOption = new BackOption;
		_options['b'] = backOption;
		backOption->setNoDelete(false);
		backOption->setIsBaseOption(true);
	}

	for (auto option : options)
		addOption(option);
}

BaseOptionContainer::~BaseOptionContainer() {
	for (auto option : _options)
		if (!option.second->getNoDelete())
			delete option.second;
}

void BaseOptionContainer::exec(IOption * parent, std::istream & input, std::ostream & output, std::string const & endline) {
	output << "Switch to " << getName() << endline << "type h to get list of available options" << endline << endline;
	char option;
	output << ">>> ";
	while (input.get(option)) {
		try {
			if (option != '\n') {
				getOption(option)->exec(this, input, output, endline);
			}
		} catch (OptionQuitException const * quit) {
			if (parent)
				throw quit;
			delete quit;
			break;
		} catch (OptionBackException const * back) {
			delete back;
			if (parent) {
				output << "Switch to " << parent->getName() << endline << "type h to get list of available options" << endline;
				break;
			}
			output << "\"" << getName() << "\" is the root container" << endline << endline;
		} catch (CliException * err) {
			output << endline << err->what() << endline;
			delete err;
		} catch (std::exception const * err) {
			std::cerr << "Error: " << err->what() << std::endl;
			delete err;
		} catch (std::exception const & err) {
			std::cerr << "Error: " << err.what() << std::endl;
		}

		if (option != '\n')
			output << endline << ">>> ";
	}

	if (!parent)
		output << "Goodbye" << endline;
	else if (input.eof())
		throw new OptionQuitException;
}

IOption * BaseOptionContainer::getOption(char name) const {
	auto option = _options.find(name);
	if (option != _options.end())
		return option->second;
	throw new OptionDoesNotExistException(name);
}

void BaseOptionContainer::addOption(char name, IOption * option) {
	if (_options.find(name) != _options.end())
		throw new OptionAlreadyExistsException(name);
	_options[name] = option;
}

void BaseOptionContainer::addOption(std::pair<char, IOption *> const & option) {
	if (_options.find(option.first) != _options.end())
		throw new OptionAlreadyExistsException(option.first);
	_options.insert(option);
}

void BaseOptionContainer::removeOption(char name) {
	if (_options.find(name) == _options.end())
		throw new OptionDoesNotExistException(name);
	delete _options[name];
	_options.erase(name);
}
