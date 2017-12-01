#ifndef XML_H
#define XML_H

#include "vecdef.h"

typedef struct
{
	const char* name;
	const char* value;
} xml_node_attr_t;

struct xml_node_t;

typedef struct xml_node_t
{
	const char* tag;
	xml_node_attr_t* attributes;
	int num_attrs;
	int num_attr_slots;
	struct xml_node_t* parent_node;
	struct xml_node_t* child_nodes;
	int num_child_nodes;
	int num_child_node_slots;
	const char* inner_text;
} xml_node_t;

typedef struct
{
	char* text;
	char* text_tok;
	size_t text_len;
	int text_pos;
	xml_node_t root_node;
	int have_root;
} xml_t;

int xml_load(xml_t* xml, const char* path);
int xml_unload(xml_t* xml);
xml_node_t* xml_root(xml_t* xml);

int xml_parser_run(xml_t* xml);
int xml_parser_is_eof(xml_t* xml);
int xml_parser_curc(xml_t* xml, char* c);
int xml_parser_inc_pos(xml_t* xml);
int xml_parser_seek_token(xml_t* xml);
int xml_parser_extract_tag_string(xml_t* xml, const char** str);
int xml_parser_extract_attribute(xml_t* xml, xml_node_attr_t* attribute);
int xml_parser_collect_attributes(xml_t* xml, xml_node_t* node);
int xml_parser_extract_inner_text(xml_t* xml, const char** text);

int xml_node_init(xml_node_t* node, const char* tag);
xml_node_t* xml_node_append_child(xml_node_t* node, xml_node_t child_node);
void xml_node_delete_child_nodes(xml_node_t* node);
int xml_node_add_attribute(xml_node_t* node, xml_node_attr_t attribute);
const char* xml_node_get_attr(xml_node_t* node, const char* name);
xml_node_t* xml_node_get_child(xml_node_t* root_node, int index);
xml_node_t* xml_node_get_by_tag(xml_node_t* root_node, const char* tag);
xml_node_t* xml_node_get_by_attr_value(xml_node_t* root_node, const char* attr_name, const char* attr_value);
xml_node_t* xml_node_get_by_id(xml_node_t* root_node, const char* id);
xml_node_t* xml_node_query(xml_node_t* root_node, const char* query);
const char* xml_node_query_attr(xml_node_t* root_node, const char* query, const char* attr_name);

int xml_node_tag_eq(xml_node_t* node, const char* tag_name);
int xml_node_attr_eq(xml_node_t* node, const char* attr_name, const char* attr_val);

int xml_node_report(xml_node_t* node);
int xml_node_report_r(xml_node_t* node, int level);

#endif // XML_H
