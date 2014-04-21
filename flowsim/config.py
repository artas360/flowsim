import xml.dom.minidom as minidom
from xml.parsers.expat import ExpatError
from flowsim.flowsim_exception import WrongConfig


def expand_list_of_list(l):
    return [item for sublist in l for item in sublist]
    

class Config:
    def __init__(self, config_src):
        self.config_src = config_src
        self.xml_config = None
        
    def read(self):
        self.openXML(self.config_src)
        self.topology_conf = []
        self.event_conf = []
        self.simulation_conf = []
        for Element in self.xml_config.getElementsByTagName('Flowsim'):
            self.topology_conf += Element.getElementsByTagName('Topology')
            self.event_conf += Element.getElementsByTagName('Events')
            self.simulation_conf += Element.getElementsByTagName('Simulation')

    def openXML(self, xmlFile):
	   try:
	  	self.xml_config = minidom.parse(xmlFile)
	   except ExpatError:
	  	raise WrongConfig
	   except IOError as ioErr:
	  	print ("(FF)", ioErr)
		raise WrongConfig

    def get_optional_attribute(self, xml_element, attribute_name, default_dict):
        if(not xml_element.getAttribute(attribute_name) == ''):
            return xml_element.getAttribute(attribute_name)
        else:
            return default_dict.get(attribute_name, None)

    def read_nodes(self, doc):
        id_list=[]
        node_list=[]
        # Nodes section
        for Nodes in doc.getElementsByTagName('Nodes'):
            default_attributes = {}
            for default in Nodes.getElementsByTagName('Default'):
                default_attributes.update(default.attributes.items())
            # Actual nodes
            for node in Nodes.getElementsByTagName('Node'):
                dic=dict()
                try:
                    dic['name']	        =	str(self.get_optional_attribute(node, 'name', default_attributes))
                    dic['service_rate']	=	float(node.getAttribute('service_rate'))
                    dic['arrival_rate']	=	float(node.getAttribute('arrival_rate'))
                    dic['_id']	=	int(node.getAttribute('id'))
                    if not dic['_id'] in id_list:
                        id_list.append(dic['_id'])
                        node_list.append(dic)
                    else:
                        print ("(EE) In the xml configuration file, duplicated node. Ignoring it.")
                        
                except IndexError:
                    print ("(EE) In the xml configuration file, missing field")
                    continue
        return (node_list, id_list)
        
    def read_links(self, doc, node_id_list):
        link_list=[]
        # Links section
        for Links in doc.getElementsByTagName('Links'):
            default_attributes = {}
            for default in Links.getElementsByTagName('Default'):
                default_attributes.update(default.attributes.items())
            # Actual Links
            for link in Links.getElementsByTagName('Link'):
                dic=dict()
                try:
                    source_id	            =	int(link.getAttribute('source_id'))
                    destination_id          =	int(link.getAttribute('destination_id'))
                    if not (source_id in node_id_list and destination_id in node_id_list):
                        print ("(EE) In the xml configuration file, link references to undeclared node")
                        continue
                    dic['nodes']            =   (source_id, destination_id)
                    dic['weight']	    =	float(self.get_optional_attribute(link, 'weight', default_attributes))
                    dic['unidir']	    =	(not self.get_optional_attribute(link, 'unidirectional', default_attributes) == "False")
                    dic['capacity']	    =	int(self.get_optional_attribute(link, 'capacity', default_attributes))
                    link_list.append(dic)
                        
                except IndexError:
                    print ("(EE) In the xml configuration file, missing field")
                    continue           
        return link_list
    def generic_leaf_read(self, parent_element, leaf_tagname):
        l = expand_list_of_list([elt.getElementsByTagName(leaf_tagname) for elt in parent_element])
        return [dict(leaf.attributes.items()) for leaf in l]
        

    def read_topology(self):
        node_id_list = []
        node_list = []
        link_list = []
        for field in self.topology_conf:
            tmp1, tmp2 = self.read_nodes(field)
            node_list += (tmp1)
            node_id_list += (tmp2)
            link_list += self.read_links(field, node_id_list)
        return node_list, link_list
        
    def read_events(self):
        return self.generic_leaf_read(self.event_conf, 'Event')
        
    def read_simulation(self):
        return self.generic_leaf_read(self.simulation_conf, 'Stop_condition')
        
