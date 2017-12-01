#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "xml.h"

int xml_load(xml_t* xml, const char* path)
{
	xml->have_root = 0;

	xml->text_pos = 0;
	
	FILE* fp = fopen(path, "rb");
	
	if(fp == NULL)
	{
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	xml->text_len = ftell(fp);
	xml->text = malloc(xml->text_len + 1);
	xml->text_tok = malloc(xml->text_len + 1);
	rewind(fp);
	
	fread(xml->text, xml->text_len, 1, fp);
	
	xml->text[xml->text_len] = '\0';
	
	memcpy(xml->text_tok, xml->text, xml->text_len + 1);
	
	return xml_parser_run(xml);
}

int xml_unload(xml_t* xml)
{
	xml_node_delete_child_nodes(&xml->root_node);
	free(xml->text);
	free(xml->text_tok);
	return 1;
}

xml_node_t* xml_root(xml_t* xml)
{
	return &xml->root_node;
}

int xml_parser_run(xml_t* xml)
{
	xml_node_t* cur_parent_node = NULL;

	while(xml_parser_seek_token(xml))
	{
		char curc;
		if(!xml_parser_curc(xml, &curc))
		{
			return 0;
		}
		
		if(curc != '<') // node must have inner text
		{
			// <tag>blah
			//      ^
			const char* inner_text = NULL;
			if(!xml_parser_extract_inner_text(xml, &inner_text))
			{
				return 0;
			}
			cur_parent_node->inner_text = inner_text;
		}
		
		xml_parser_inc_pos(xml);
		
		if(!xml_parser_curc(xml, &curc))
		{
			return 0;
		}
		
		if(curc == '?')
		{
			// <?xml etc>
			//  ^
			xml_parser_inc_pos(xml);
			
			while(curc != '>') // just skip
			{
				xml_parser_inc_pos(xml);
				if(!xml_parser_curc(xml, &curc))
				{
					return 0;
				}
			}
			
			xml_parser_inc_pos(xml);
		}
		else if(curc == '/')
		{
			// <tag></tag>
			//       ^
			xml_parser_inc_pos(xml);
			const char* tag;
			xml_parser_extract_tag_string(xml, &tag);
			xml_parser_inc_pos(xml);
			// todo compare end tag with start tag for sanity
			cur_parent_node = cur_parent_node->parent_node;
		}
		else
		{
			const char* tag;
			if(!xml_parser_extract_tag_string(xml, &tag))
			{
				return 0;
			}
			
			xml_node_t node;
			xml_node_init(&node, tag);
			
			xml_parser_collect_attributes(xml, &node);
			
			xml_parser_curc(xml, &curc);
			
			if(curc == '>' || curc == '/')
			{
				if(!xml->have_root)
				{
					xml->root_node = node;
					cur_parent_node = &xml->root_node;
					xml->have_root = 1;
				}
				else
				{
					cur_parent_node = xml_node_append_child(cur_parent_node, node);
				}
				
				if(curc == '/') // tag without closing tag
				{
					cur_parent_node = cur_parent_node->parent_node;
					xml_parser_inc_pos(xml);
				}

				// okay
				xml_parser_inc_pos(xml);
			}
		}
	}
	
	return 1;
}

int xml_parser_is_eof(xml_t* xml)
{
	return xml->text_pos >= xml->text_len;
}

int xml_parser_curc(xml_t* xml, char* c)
{
	if(xml_parser_is_eof(xml))
	{
		return 0;
	}
	*c = xml->text[xml->text_pos];
	return 1;
}

int xml_parser_inc_pos(xml_t* xml)
{
	if(xml_parser_is_eof(xml))
	{
		return 0;
	}
	xml->text_pos++;
	return 1;
}

int xml_parser_seek_token(xml_t* xml)
{
	char c;
	
	while(xml_parser_curc(xml, &c))
	{
		if(!isspace(c))
		{
			return 1;
		}
		xml_parser_inc_pos(xml);
	}
	return 0;
}

int xml_parser_extract_tag_string(xml_t* xml, const char** str)
{
	int curpos = xml->text_pos;
	
	char curc;
	while(xml_parser_curc(xml, &curc))
	{
		if(isspace(curc) || curc == '>' || curc == '/')
		{
			*str = &xml->text_tok[curpos];
			xml->text_tok[xml->text_pos] = 0;
			return 1;
		}
		xml_parser_inc_pos(xml);
	}
	return 0;
}

int xml_parser_extract_attribute(xml_t* xml, xml_node_attr_t* attribute)
{
	int curpos = xml->text_pos;
	char curc;
	
	while(xml_parser_curc(xml, &curc))
	{
		if(isspace(curc) || curc == '=')
		{
			attribute->name = &xml->text_tok[curpos];
			xml->text_tok[xml->text_pos] = '\0';
			xml_parser_inc_pos(xml);
			break;
		}
		xml_parser_inc_pos(xml);
	}
	
	xml_parser_seek_token(xml);
	if(!xml_parser_curc(xml, &curc))
	{
		return 0;
	}
	
	if(curc != '\"')
	{
		return 0;
	}
	
	xml_parser_inc_pos(xml);
	curpos = xml->text_pos;
	
	while(xml_parser_curc(xml, &curc))
	{
		if(curc == '"')
		{
			attribute->value = &xml->text_tok[curpos];
			xml->text_tok[xml->text_pos] = '\0';
			xml_parser_inc_pos(xml);
			return 1;
		}
		xml_parser_inc_pos(xml);
	}
	
	return 0;
}

int xml_parser_collect_attributes(xml_t* xml, xml_node_t* node)
{
	char curc;
	// fetch attributes
	while(xml_parser_seek_token(xml))
	{
		if(!xml_parser_curc(xml, &curc))
		{
			return 0;
		}
		
		if(curc == '>' || curc == '/')
		{
			break;
		}
		else
		{
			xml_parser_seek_token(xml);
			
			xml_node_attr_t attribute;
			xml_parser_extract_attribute(xml, &attribute);
			
			xml_node_add_attribute(node, attribute);
		}
	}
	return 1;
}

int xml_parser_extract_inner_text(xml_t* xml, const char** text)
{
	int start_pos = xml->text_pos;

	char curc;
	while(xml_parser_curc(xml, &curc))
	{
		if(curc == '<')
		{
			xml->text_tok[xml->text_pos] = '\0';
			//xml_parser_inc_pos(xml);
			*text = &xml->text_tok[start_pos];
			//xml_parser_inc_pos(xml);
			return 1;
		}
		xml_parser_inc_pos(xml);
	}
	
	return 1;
}

int xml_node_init(xml_node_t* node, const char* tag)
{
	node->tag = tag;
	node->attributes = NULL;
	node->num_attrs = 0;
	node->num_attr_slots = 0;
	node->parent_node = NULL;
	node->child_nodes = NULL;
	node->num_child_nodes = 0;
	node->num_child_node_slots = 0;
	node->inner_text = NULL;
	
	return 1;
}

xml_node_t* xml_node_append_child(xml_node_t* node, xml_node_t child_node)
{
	child_node.parent_node = node;

	if(node->num_child_nodes + 1 > node->num_child_node_slots)
	{
		if(node->num_child_node_slots == 0)
		{
			node->num_child_node_slots = 1;
			node->child_nodes = malloc(sizeof(xml_node_t));
		}
		else
		{
			node->num_child_node_slots *= 2;
			node->child_nodes = realloc(node->child_nodes, node->num_child_node_slots * sizeof(xml_node_t));
		}	
	}

	node->child_nodes[node->num_child_nodes] = child_node;
	xml_node_t* added_node = &node->child_nodes[node->num_child_nodes];
	node->num_child_nodes++;
	return added_node;
}

void xml_node_delete_child_nodes(xml_node_t* node)
{
	if(node->num_attrs > 0)
	{
		node->num_attrs = 0;
		node->num_attr_slots = 0;

		free(node->attributes);
	}

	if(node->num_child_nodes == 0)
	{
		return;
	}

	for(int i = 0; i < node->num_child_nodes; i++)
	{
		xml_node_delete_child_nodes(&node->child_nodes[i]);
	}

	node->num_child_nodes = 0;
	node->num_child_node_slots = 0;

	free(node->child_nodes);
}

int xml_node_add_attribute(xml_node_t* node, xml_node_attr_t attribute)
{
	if(node->num_attrs + 1 > node->num_attr_slots)
	{
		if(node->num_attr_slots == 0)
		{
			node->num_attr_slots = 1;
			node->attributes = malloc(sizeof(xml_node_attr_t));
		}
		else
		{
			node->num_attr_slots *= 2;
			node->attributes = realloc(node->attributes, node->num_attr_slots * sizeof(xml_node_attr_t));
		}	
	}
	node->attributes[node->num_attrs++] = attribute;
	return node->num_attrs;
}

const char* xml_node_get_attr(xml_node_t* node, const char* name)
{
	for(int i = 0; i < node->num_attrs; i++)
	{
		if(strcmp(name, node->attributes[i].name) == 0)
		{
			return node->attributes[i].value;
		}
	}
	return NULL;
}

xml_node_t* xml_node_get_child(xml_node_t* root_node, int index)
{
	if(index >= root_node->num_child_nodes)
	{
		return NULL;
	}
	return &root_node->child_nodes[index];
}

xml_node_t* xml_node_get_by_tag(xml_node_t* root_node, const char* tag)
{
	if(strcmp(root_node->tag, tag) == 0)
	{
		return root_node;
	}

	for(int i = 0; i < root_node->num_child_nodes; i++)
	{
		xml_node_t* node = xml_node_get_by_tag(&root_node->child_nodes[i], tag);
		if(node != NULL)
		{
			return node;
		}
	}

	return NULL;
}

// recursive search for node by arbitrary attribute value
xml_node_t* xml_node_get_by_attr_value(xml_node_t* root_node, const char* attr_name, const char* attr_value)
{
	const char* node_id_val = xml_node_get_attr(root_node, attr_name);

	if(node_id_val != NULL && strcmp(node_id_val, attr_value) == 0)
	{
		return root_node;
	}

	for(int i = 0; i < root_node->num_child_nodes; i++)
	{
		xml_node_t* node = xml_node_get_by_attr_value(&root_node->child_nodes[i], attr_name, attr_value);
		if(node != NULL)
		{
			return node;
		}
	}

	return NULL;
}

// recursive search for node by "id" attribute value
xml_node_t* xml_node_get_by_id(xml_node_t* root_node, const char* id)
{
	return xml_node_get_by_attr_value(root_node, "id", id);
}

xml_node_t* xml_node_query(xml_node_t* root_node, const char* query)
{
	char cur_directive = 0;
	int qi = 0;
	int qlen = strlen(query);
	char* q = malloc(qlen + 1); // tokenizable copy
	strcpy(q, query);
	q[qlen] = '\0';

	while(qi < qlen)
	{
		for(; qi < qlen && query[qi] == ' '; qi++); // seek token
		
		if(qi == qlen)
		{
			break; // reached end of line
		}
		
		char curc = query[qi];
	
		switch(curc) // check for directive character
		{
		case '#':
			cur_directive = '#'; // id attribute match
			qi++;
			break;
		default:
			cur_directive = 0; // default, tag name match
			break;
		}

		// extract token string
		char* token = &q[qi];
		for(; qi < qlen && query[qi] != ' '; qi++); // seek end of token
		q[qi] = '\0'; // terminate token
		qi++;

		switch(cur_directive)
		{
		case '#':
			root_node = xml_node_get_by_id(root_node, token);
			break;
		case 0:
			root_node = xml_node_get_by_tag(root_node, token);
			break;
		}

		if(root_node == NULL)
		{
			free(q);
			return NULL;
		}
	}

	free(q);
	return root_node;
}

const char* xml_node_query_attr(xml_node_t* root_node, const char* query, const char* attr_name)
{
	xml_node_t* node = xml_node_query(root_node, query);
	
	if(node == NULL)
	{
		return NULL;
	}

	return xml_node_get_attr(node, attr_name);
}

int xml_node_report_r(xml_node_t* node, int level)
{
	for(int i = 0; i < level; i++)
		printf("- ");

	printf("<%s", node->tag);

	if(node->num_attrs > 0)
	{
		for(int i = 0; i < node->num_attrs; i++)
		{
			printf(" %s=\"%s\"", node->attributes[i].name, node->attributes[i].value);
		}
	}

	if(node->num_child_nodes == 0 && node->inner_text == NULL)
	{
		printf("/>\n");
	}
	else
	{
		printf(">\n");
		
		if(node->inner_text != NULL)
		{
			for(int i = 0; i < level + 1; i++) printf("  ");
			printf("%s\n", node->inner_text);
		}
	
		for(int i = 0; i < node->num_child_nodes; i++)
		{
			xml_node_report_r(&node->child_nodes[i], level + 1);
		}
	
		for(int i = 0; i < level; i++)
			printf("  ");
	
		printf("</%s>\n", node->tag);
	}
	
	return 1;
}

int xml_node_tag_eq(xml_node_t* node, const char* tag_name)
{
	if(strcmp(node->tag, tag_name) == 0)
	{
		return 1;
	}
	return 0;
}

int xml_node_attr_eq(xml_node_t* node, const char* attr_name, const char* attr_val)
{
	const char* val = xml_node_get_attr(node, attr_name);
	if(val == NULL)
	{
		return 0;
	}
	if(strcmp(attr_val, val) == 0)
	{
		return 1;
	}
	return 0;
}

int xml_node_report(xml_node_t* node)
{
	xml_node_report_r(node, 0);
	printf("----------------------\n");
	return 1;
}