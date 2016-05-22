#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "t3net.h"
#include "internal.h"

static void t3net_destroy_data_entry_field(T3NET_DATA_ENTRY_FIELD * field);
static void t3net_destroy_data_entry(T3NET_DATA_ENTRY * entry);

char t3net_server_message[1024] = {0};

void t3net_strcpy(char * dest, const char * src, int size)
{
	int c = 1;
	int pos = 0;

	while(c != '\0')
	{
		c = src[pos];
		dest[pos] = c;
		pos++;
		if(pos >= size)
		{
			pos = size - 1;
			break;
		}
	}
	dest[pos] = '\0';
}

void t3net_strcpy_url(char * dest, const char * src, int size)
{
	int write_pos = 0;
	int c = 1;
	int pos = 0;

	while(c != '\0')
	{
		c = src[pos];
		if(c == ' ' && write_pos + 4 < size)
		{
			dest[write_pos] = '%';
			write_pos++;
			dest[write_pos] = '2';
			write_pos++;
			dest[write_pos] = '0';
			write_pos++;
		}
		else
		{
			dest[write_pos] = c;
			write_pos++;
		}
		pos++;
	}
}

char * t3net_strcat(char * dest, const char * src, int size)
{
	int write_pos = -1;
	int c = 1;
	int pos = 0;

	/* find end of destination string */
	while(c != '\0')
	{
		write_pos++;
		c = dest[write_pos];
	}

	c = 1;
	/* concatenate src onto dest */
	while(c != '\0')
	{
		c = src[pos];
		dest[write_pos] = c;
		pos++;
		write_pos++;
		if(write_pos >= size)
		{
			write_pos = size - 1;
		}
	}
	dest[write_pos] = '\0';
	return dest;
}

typedef struct
{

	char * data;
	size_t filled;

} T3NET_MEMORY_CHUNK;

size_t t3net_internal_write_function(void * ptr, size_t size, size_t nmemb, void * stream)
{
	size_t realsize = size * nmemb;
	T3NET_MEMORY_CHUNK * mem = (T3NET_MEMORY_CHUNK *)stream;
	size_t blocks = (mem->filled + 1) / T3NET_DATA_CHUNK_SIZE + 1;
	size_t blocks_required = (realsize + mem->filled + 1) / T3NET_DATA_CHUNK_SIZE + 1;

	/* increase chunk size if we exceed it */
	if(realsize + mem->filled + 1 >= T3NET_DATA_CHUNK_SIZE * blocks)
	{
		mem->data = realloc(mem->data, T3NET_DATA_CHUNK_SIZE * blocks_required);
		if(mem->data == NULL)
		{
  	    	/* out of memory! */
  	    	return 0;
		}
	}
	memcpy(&(mem->data[mem->filled]), ptr, realsize);
	mem->filled += realsize;
	mem->data[mem->filled] = '\0';

	return realsize;
}

static int t3net_get_line_length(const char * data, unsigned int text_pos)
{
	int length = 0;
	int c;

	while(1)
	{
		c = data[text_pos];
		if(c != '\r')
		{
			length++;
		}
		else if(c == '\0')
		{
			return length;
		}
		else
		{
			text_pos++;
			c = data[text_pos];
			if(c != '\n')
			{
				return -1;
			}
			else
			{
				return length;
			}
		}
		text_pos++;
	}
	return -1;
}

/* Read a line of text and put it in a separate buffer. Lines are expected to end in "\r\n". */
int t3net_read_line(const char * data, char * output, int data_max, int output_max, unsigned int * text_pos)
{
	int outpos = 0;
	int c;

	while(1)
	{
		c = data[*text_pos];
		if(c != '\r')
		{
			output[outpos] = c;
		}
		else
		{
			output[outpos] = '\0';
			(*text_pos)++;
			c = data[*text_pos];
			if(c == '\n')
			{
				(*text_pos)++;
				return 1;
			}
			else
			{
				return 0;
			}
		}
		outpos++;
		if(outpos >= output_max)
		{
			outpos--;
			output[outpos] = '\0';
			return 1;
		}
		(*text_pos)++;
		if(*text_pos >= data_max)
		{
			return 0;
		}
	}
	return 0;
}

char * t3net_get_line(const char * data, int data_max, unsigned int * text_pos)
{
	char * text_line = NULL;
	int bytes = 0;

	bytes = t3net_get_line_length(data, *text_pos) + 1;
	if(bytes > 0)
	{
		text_line = malloc(bytes);
	}
	if(text_line)
	{
		t3net_read_line(data, text_line, data_max, bytes, text_pos);
	}
	return text_line;
}

int t3net_get_element(const char * data, T3NET_TEMP_ELEMENT * element, int data_max)
{
	int outpos = 0;
	int c;
	int read_pos = 1; // skip first byte

	/* read element name */
	while(1)
	{
		c = data[read_pos];

		if(c == ':')
		{
			read_pos += 2;
			break;
		}
		else
		{
			element->name[outpos] = c;
			outpos++;
			element->name[outpos] = '\0';
			read_pos++;
		}
	}

	/* read element data */
	outpos = 0;
	while(c != '\0' && read_pos < data_max)
	{
		c = data[read_pos];

		element->data[outpos] = c;
		outpos++;
		element->data[outpos] = '\0';
		read_pos++;
	}
	return 1;
}

char * t3net_get_raw_data(const char * url)
{
	CURL * curl;
	T3NET_MEMORY_CHUNK data;

	data.data = malloc(T3NET_DATA_CHUNK_SIZE);
	if(!data.data)
	{
		return NULL;
	}
	memset(data.data, 0, T3NET_DATA_CHUNK_SIZE);
	data.filled = 0;

	/* make HTTP request */
	curl = curl_easy_init();
	if(!curl)
	{
		free(data.data);
		return NULL;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_internal_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, T3NET_TIMEOUT_TIME);
    if(curl_easy_perform(curl))
    {
		curl_easy_cleanup(curl);
		free(data.data);
		return NULL;
	}
    curl_easy_cleanup(curl);

    return data.data;
}

static int t3net_count_data_entries_in_string(const char * s, int * field_max)
{
	int c = -1;
	int pos = 0;
	int step = 0;
	int entries = -1;
	int fields = 0;

	*field_max = 0;
	while(c != '\0')
	{
		c = s[pos];
		if(step == 0)
		{
			if(c == '\r')
			{
				step++;
			}
		}
		else if(step == 1)
		{
			if(c == '\n')
			{
				step++;
			}
			else
			{
				step = 0;
			}
		}
		else if(step == 2)
		{
			if(c == '\r')
			{
				step++;
			}
			else
			{
				fields++;
				step = 0;
			}
		}
		else if(step == 3)
		{
			if(c == '\n')
			{
				entries++;
				if(fields > *field_max)
				{
					*field_max = fields;
				}
				fields = 1;
			}
			step = 0;
		}
		pos++;
	}
	return entries;
}

static T3NET_DATA_ENTRY * t3net_create_data_entry(int max_fields)
{
	T3NET_DATA_ENTRY * entry;
	int i;

	entry = malloc(sizeof(T3NET_DATA_ENTRY));
	if(!entry)
	{
		goto error_out;
	}
	entry->field = malloc(sizeof(T3NET_DATA_ENTRY_FIELD *) * max_fields);
	if(!entry->field)
	{
		goto error_out;
	}
	memset(entry->field, 0, sizeof(T3NET_DATA_ENTRY_FIELD *) * max_fields);
	for(i = 0; i < max_fields; i++)
	{
		entry->field[i] = malloc(sizeof(T3NET_DATA_ENTRY_FIELD));
		if(!entry->field[i])
		{
			goto error_out;
		}
		memset(entry->field[i], 0, sizeof(T3NET_DATA_ENTRY_FIELD));
		entry->fields = max_fields;
	}
	return entry;

	error_out:
	{
		t3net_destroy_data_entry(entry);
		return NULL;
	}
}

static T3NET_DATA * t3net_create_data(int max_entries, int max_fields)
{
	T3NET_DATA * data;
	int i;

	data = malloc(sizeof(T3NET_DATA));
	if(!data)
	{
		return NULL;
	}
	memset(data, 0, sizeof(T3NET_DATA));

	/* allocate space for data entries */
	data->entry = NULL;
	if(max_entries > 0)
	{
		data->entry = malloc(sizeof(T3NET_DATA_ENTRY *) * max_entries);
		if(!data->entry)
		{
			goto error_out;
		}
		for(i = 0; i < max_entries; i++)
		{
			data->entry[i] = t3net_create_data_entry(max_fields);
			if(!data->entry[i])
			{
				goto error_out;
			}
		}
	}
	data->entries = max_entries;
	return data;

	error_out:
	{
		t3net_destroy_data(data);
		return NULL;
	}
}

T3NET_DATA * t3net_get_data_from_string(const char * raw_data)
{
	int text_max;
	T3NET_DATA * data = NULL;
	char * current_line;
	int entries = 0;
	int fields = 0;
	int l, size;
	int field = 0;
	unsigned int text_pos = 0;
	int ecount = -1;
	T3NET_TEMP_ELEMENT element;

	if(!raw_data)
	{
		return NULL;
	}

	/* check for error */
	if(!strncmp(raw_data, "Error", 5))
	{
		return NULL;
	}

	entries = t3net_count_data_entries_in_string(raw_data, &fields);
	if(entries < 0)
	{
		goto error_out;
	}
	data = t3net_create_data(entries, fields);
	if(!data)
	{
		return NULL;
	}

	text_pos = 0;
    text_max = strlen(raw_data) + 1;

    /* read header */
	current_line = t3net_get_line(raw_data, text_max, &text_pos);
	if(!current_line)
	{
		goto error_out;
	}
	data->header = current_line;

	while(1)
	{
		current_line = t3net_get_line(raw_data, text_max, &text_pos);
		if(current_line)
		{
			l = strlen(current_line);
			if(l <= 0)
			{
				ecount++;
				field = 0;
			}
			else
			{
				size = l + 1;
				t3net_get_element(current_line, &element, size);

				/* copy field name */
				size = strlen(element.name) + 1;
				if(size > 0)
				{
					data->entry[ecount]->field[field]->name = malloc(size);
					if(data->entry[ecount]->field[field]->name)
					{
						t3net_strcpy(data->entry[ecount]->field[field]->name, element.name, size);
					}
				}

				/* copy field data */
				size = strlen(element.data) + 1;
				if(size > 0)
				{
					data->entry[ecount]->field[field]->data = malloc(size);
					if(data->entry[ecount]->field[field]->data)
					{
						t3net_strcpy(data->entry[ecount]->field[field]->data, element.data, size);
					}
				}
				field++;
			}
		}
		else
		{
			break;
		}

		/* get out if we've reached the end of the data */
		if(text_pos >= text_max)
		{
			break;
		}
	}
	return data;

	error_out:
	{
		t3net_destroy_data(data);
		return NULL;
	}
}

T3NET_DATA * t3net_get_data(const char * url)
{
	char * raw_data;
	T3NET_DATA * data = NULL;

	raw_data = t3net_get_raw_data(url);
	if(raw_data)
	{
		data = t3net_get_data_from_string(raw_data);
		free(raw_data);
	}
	return data;
}

static void t3net_destroy_data_entry_field(T3NET_DATA_ENTRY_FIELD * field)
{
	if(field)
	{
		if(field->name)
		{
			free(field->name);
		}
		if(field->data)
		{
			free(field->data);
		}
		free(field);
	}
}

static void t3net_destroy_data_entry(T3NET_DATA_ENTRY * entry)
{
	int j;

	if(entry)
	{
		if(entry->field)
		{
			for(j = 0; j < entry->fields; j++)
			{
				t3net_destroy_data_entry_field(entry->field[j]);
			}
			free(entry->field);
		}
		free(entry);
	}
}

void t3net_destroy_data(T3NET_DATA * data)
{
	int i;

	if(data)
	{
		if(data->entry)
		{
			for(i = 0; i < data->entries; i++)
			{
				t3net_destroy_data_entry(data->entry[i]);
			}
			free(data->entry);
		}
		free(data);
	}
}

const char * t3net_get_data_entry_field(T3NET_DATA * data, int entry, const char * field_name)
{
	int i;

	if(entry < data->entries)
	{
		for(i = 0; i < data->entry[entry]->fields; i++)
		{
			if(!strcmp(data->entry[entry]->field[i]->name, field_name))
			{
				return data->entry[entry]->field[i]->data;
			}
		}
	}
	return NULL;
}