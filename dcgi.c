/* dcgi.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "dcgi.h"



static char hexcharconvert( char hexchar)
{
  return (hexchar >= 'A' ? (hexchar & 0xdf) - 'A' + 10 : hexchar - '0');
}

/* returns 1 on error */
static int unescape_url(char *url)
{
  char *p, *q;

  for( p = q = url; *q != '\0'; p++, q++)
    {
      if( *q == '%')
	{
	  if( !isxdigit( *(q+1)) || !isxdigit( *(q+2)) )
	    return 1;

          *p = 16*hexcharconvert(*(q+1)) + hexcharconvert(*(q+2));
          q += 2;
	}
      else
        *p = *q;
    }
  *p = '\0';

  return 0;
}

static void unplus_url(char *url)
{
  while( *url)
    {
      if( *url == '+')
	*url = ' ';
      url++;
    }
}


static int pair_compare (const void * a, const void * b)
{
  struct cgi_pair *pair_a = (struct cgi_pair *) a;
  struct cgi_pair *pair_b = (struct cgi_pair *) b;

  return strcmp( pair_a->key, pair_b->key);
}


const char *cgi_get_error_string( int error)
{
  char *cgi_error[7] = 
    { 
      /* 0 */  "No data sent.",                                    
      /* 1 */  "There is no REQUEST_METHOD defined.",
      /* 2 */  "The REQUEST_METHOD is invalid.",
      /* 3 */  "There is no CONTENT_LENGTH defined.",
      /* 4 */  "The input length is less than CONTENT_LENGTH.",
      /* 5 */  "The input data is ill formed.",
      /* 6 */  "There is not enough memory."
    };

  if( (error > 0) || (error < -6))
    return "Bad error value";
  
  return cgi_error[-error];
}


/* Returns negative number on error, else is gives the number of fields read.
   One can print an error message by taking the negative of the return value,
   making it positive, and printing cgi_error[value] out.  */
int cgi_parse_input(struct cgi_info *info)
{
  int number_pairs = 0;
  struct cgi_pair *pairs = NULL;

  int input_length;
  char *input_buffer = NULL;

  char *p;
  int i;
  
  info->number_pairs = 0;
  info->pairs = NULL;

  p = getenv( "REQUEST_METHOD");
  if( p == NULL)
    return -1;
  if( !strcmp( p, "POST") )
    {
      p = getenv("CONTENT_LENGTH");
      if( p == NULL)
	return -3;
      input_length = atoi( p);
      if( !input_length)
        return 0;
      input_buffer = malloc( sizeof(char)*(input_length+1));
      if( !input_buffer)
	return -6;
      if( fread(input_buffer, sizeof(char), input_length, stdin) != 
	  input_length)
	return -4;
      input_buffer[input_length] = '\0';
    }
  else if( !strcmp( p, "GET"))
    {
      p = getenv( "QUERY_STRING");
      if( p == NULL)
        return 0;
      input_length = strlen( p);
      if( !input_length)
        return 0;
      input_buffer = malloc( sizeof(char)*(input_length+1));
      if( !input_buffer)
	return -6;
      strcpy( input_buffer, p);
    }
  else
    return -2;

  number_pairs = 1;
  for( p = input_buffer; (p = strchr( p, '&')) != NULL ; p++ )
    number_pairs++;
  pairs = malloc( sizeof( struct cgi_pair) * number_pairs);
  if( !pairs )
    return -6;
  
  pairs[0].key = input_buffer;
  for( i = 1; i < number_pairs; i++)
    {
      p = strchr( pairs[i-1].key, '&');
      *p = '\0';
      pairs[i].key = p+1;
    }
  for( i = 0; i < number_pairs; i++)
    {
      p = strchr( pairs[i].key, '=');
      if( p == NULL)
	return -5;
      *p = '\0';
      pairs[i].value = p+1;
      if( strchr( pairs[i].value, '=') )
	return -5;
    }

  for( i = 0; i < number_pairs; i++)
    {
      unplus_url( pairs[i].key);
      unplus_url( pairs[i].value);

      if( unescape_url( pairs[i].key) )
	return -5;
      if( unescape_url( pairs[i].value) )
	return -5;

      pairs[i].key = strdup(pairs[i].key);
      pairs[i].value = strdup(pairs[i].value);
    }
  free( input_buffer);

  qsort(pairs, number_pairs, sizeof( struct cgi_pair), pair_compare);

  info->number_pairs = number_pairs;
  info->pairs = pairs;

  /* for( i = 0; i < info->number_pairs; i++) */
  /*   printf(" %s = %s\n", info->pairs[i].key, info->pairs[i].value); */
  
  return number_pairs;
}


/* this is a binary search */
char *cgi_get_value( struct cgi_info *info, char *key)
{
  int high = info->number_pairs - 1;
  int low = 0;
  int mid;
  int result;

  if( !info->number_pairs)
    return NULL;

  while(1)
    {
      mid = (high + low)/2;

      result = strcmp( key, info->pairs[mid].key);
      if( !result )
        return info->pairs[mid].value;
      else 
	{
	  if( result < 0)
	    high = mid - 1;
	  else
	    low = mid + 1;
	}

      if( high < low)
	break;
    }
  return NULL;
}

void cgi_clean_up(struct cgi_info *info)
{
  int i;

  for( i = 0; i < info->number_pairs; i++)
    {
      free( info->pairs[i].key);
      free( info->pairs[i].value);
    }
  free( info->pairs);

  info->number_pairs = 0;
  info->pairs = NULL;
}

