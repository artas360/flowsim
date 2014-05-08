#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

using namespace xercesc;

typedef std::unordered_map<std::string, std::string> parameter_map;
typedef std::vector<parameter_map> config_list;

void generic_parse(DOMNodeList*, config_list&);
void parse_nodes(DOMNodeList*, config_list&);
void parse_links(DOMNodeList*, config_list&);
void parse_events(DOMNodeList*, config_list&);
void parse_simulation(DOMNodeList*, config_list&);
int parse_config(const char* filename, config_list &, config_list &, config_list &, config_list &);

int parse_config(const char* filename,
                 config_list &node_list,
                 config_list &link_list,
                 config_list &event_list,
                 config_list &simulation_list) {
    char buf[100];

    try { XMLPlatformUtils::Initialize(); }
    catch (const XMLException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        std::cerr << "Error during initialization! :\n"
            << message << "\n";
        XMLString::release(&message);
            return EXIT_FAILURE;
    }

    XercesDOMParser* parser = new XercesDOMParser();
    parser->setValidationScheme(XercesDOMParser::Val_Auto);
    parser->setDoNamespaces(false);
    parser->setDoSchema(false);
    parser->setValidationConstraintFatal(false);

    XMLCh *f = XMLString::transcode(filename);

    parser->parse(f);
    XMLString::release(&f);

    DOMElement* docRootNode;
    DOMDocument* doc;
    DOMTreeWalker* walker;
    doc = parser->getDocument();
    docRootNode = doc->getDocumentElement();

    try {
        walker = doc->createTreeWalker(docRootNode, DOMNodeFilter::SHOW_ELEMENT, nullptr, true);
    } catch (xercesc_3_1::DOMException const& e) {
        std::cerr << "DOM_EXCEPTION" << std::endl;
        std::cerr << "Check if DTD path is right in " << filename << std::endl;
        return EXIT_FAILURE;
    }

    DOMNode * current_node = walker->nextNode();

    while(current_node != nullptr) {
        XMLString::transcode(current_node->getNodeName(), buf, 99);
        if(strncmp(buf, "Topology", 9) == 0) {
            DOMTreeWalker* local = doc->createTreeWalker(walker->getCurrentNode(), DOMNodeFilter::SHOW_ELEMENT, nullptr, true);
            for (DOMNode* current_child = local->nextNode(); current_child != nullptr; current_child = local->nextNode()) {
                XMLString::transcode(current_child->getNodeName(), buf, 99);
                if(strncmp(buf, "Links", 6) == 0) {
                    parse_links(current_child->getChildNodes(), link_list);
                }
                else if(strncmp(buf, "Nodes", 6) == 0) {
                    parse_nodes(current_child->getChildNodes(), node_list);
                }
            }
            current_node = walker->nextSibling();
        }
        else if(strncmp(buf, "Events", 7) == 0) {
            parse_events(current_node->getChildNodes(), event_list);
            current_node = walker->nextSibling();
        }
        else if(strncmp(buf, "Simulation", 11) == 0) {
            parse_simulation(current_node->getChildNodes(), simulation_list);
            current_node = walker->nextSibling();
        }
        else {
            current_node = walker->nextNode();
        }
    }

    delete parser;
    XMLPlatformUtils::Terminate();
    return EXIT_SUCCESS;
}


void generic_parse(DOMNodeList* node_list, config_list& conf) {
    assert(node_list != nullptr);
    DOMNode* current_child = nullptr;
    parameter_map map;
    char child_name[100], param_name[100], value[100];
    for (XMLSize_t i = 0; i < node_list->getLength(); ++i) {
        map.clear();
        current_child = node_list->item(i);
        XMLString::transcode(current_child->getNodeName(), child_name, 99);
        if(child_name[0] == '#')
            continue;
        if(strncmp(child_name, "Default", 8) == 0)
            continue;  // TODO: Do something useful
        for(XMLSize_t i = 0; i < current_child->getAttributes()->getLength(); ++i) {
            XMLString::transcode(current_child->getAttributes()->item(i)->getNodeName(), param_name, 99);
            XMLString::transcode(current_child->getAttributes()->item(i)->getNodeValue(), value, 99);
            map[std::string(param_name)] = std::string(value);
        }
        conf.push_back(map);
    }
    // for(auto it1: conf) {
    //     for(auto it2: it1) {
    //         std::cerr << it2.first << " " << it2.second << std::endl;
    //     }
    //     std::cerr << std::endl << std::endl;
    // }
    // std::cerr << std::endl << std::endl;
}

void parse_nodes(DOMNodeList* node_list, config_list& conf) {
    generic_parse(node_list, conf);
}

void parse_links(DOMNodeList* node_list, config_list& conf) {
    generic_parse(node_list, conf);
}

void parse_events(DOMNodeList* node_list, config_list& conf) {
    generic_parse(node_list, conf);
}

void parse_simulation(DOMNodeList* node_list, config_list& conf) {
    generic_parse(node_list, conf);
}


#if TEST_CONFIG

int main() {
    config_list node_list,
                link_list,
                event_list,
                simulation_list;

    return parse_config("../../config/config-sample.xml",
                        node_list,
                        link_list,
                        event_list,
                        simulation_list);
}

#endif

#endif
