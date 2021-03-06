#include <stdio.h>
#include <stdlib.h>
#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <microhttpd.h>

#include "define_macro_functions.h"
#include "define_macros.h"
#include "structures.h"
#include "variables.h"

#include "define_macros_functions.h"
#include "database_functions.h"
#include "server_functions.h"
#include "string_functions.h"

/*
-----------------------------------------------------------------------------------------------------------
Functions
-----------------------------------------------------------------------------------------------------------
*/

/*
-----------------------------------------------------------------------------------------------------------
Name: read_file_server
Description: Reads a file for the server
Parameters:
   cls - Callback value
   position - Position to set the file pointer to
   data - Holds the file fata
   file_size - The size of the file
Return: The number of bytes read from the file
-----------------------------------------------------------------------------------------------------------
*/

ssize_t read_file_server(void *cls, size_t position , char* data, size_t file_size)
{
  FILE* file = cls;
  // set the pointer to the begining of the file
  fseek(file, position, SEEK_SET);
  // read the file into the buffer
  const ssize_t file_data = fread(data, sizeof(char), file_size, file);
  return file_data;
}



/*
-----------------------------------------------------------------------------------------------------------
Name: callback_reset
Description: resets the file read by the server
Parameters:
   cls - Callback value
-----------------------------------------------------------------------------------------------------------
*/

void callback_reset(void *cls)
{
  // close the file
  FILE* file = cls;
  fclose(file);
}



/*
-----------------------------------------------------------------------------------------------------------
Name: post_result
Description: Gets the data from the post request
Parameters:
   connection_info_cls - Callback value
   post_type - The post data item type
   item - The post data item
   file_name - file name
   content_type - Content type
   transfer_encoding - The post data item encoding
   data - The post data item value
   position - The position of the data
   size - The post data value size
Return: 0 if an error has occured, 1 if successfull
-----------------------------------------------------------------------------------------------------------
*/

int post_result(void *connection_info_cls, enum MHD_ValueKind post_type, const char* item,const char* file_name, const char* content_type,const char* transfer_encoding, const char* data, uint64_t position, size_t size)
{
  // Variables
  struct connection_info_struct *connection_info = connection_info_cls;
  char* result = malloc(POST_DATA_MAXIMUM_ITEM_SIZE);
  int settings;

  // define macros
  #define GET_DATA(item_data) \
  sprintf(result, "%s", data); \
  decode_html(result); \
  connection_info->item[connection_info->count] = item_data; \
  connection_info->value[connection_info->count] = result; \
  connection_info->count = connection_info->count + 1; \
  settings = MHD_YES;

  // error check
  if (size > 0 && size <= POST_DATA_MAXIMUM_ITEM_SIZE)
  {
    if (strncmp(item,"public_address",BUFFER_SIZE) == 0)
    {
      GET_DATA("public_address");
    }
    else if (strncmp(item,"delegate_name",BUFFER_SIZE) == 0)
    {
      GET_DATA("delegate_name");
    }
    else if (strncmp(item,"IP_address",BUFFER_SIZE) == 0)
    {
      GET_DATA("IP_address");
    }
    else if (strncmp(item,"pool_mode",BUFFER_SIZE) == 0)
    {
     GET_DATA("pool_mode");
    }
    else if (strncmp(item,"password",BUFFER_SIZE) == 0)
    {
      GET_DATA("password");
    }
    else if (strncmp(item,"session",BUFFER_SIZE) == 0)
    {
      GET_DATA("session");
    }
    else if (strncmp(item,"about",BUFFER_SIZE) == 0)
    {
      GET_DATA("about");
    }
    else if (strncmp(item,"website",BUFFER_SIZE) == 0)
    {
      GET_DATA("website");
    }
    else if (strncmp(item,"team",BUFFER_SIZE) == 0)
    {
      GET_DATA("team");
    }
    else if (strncmp(item,"fee_structure",BUFFER_SIZE) == 0)
    {
      GET_DATA("fee_structure");
    }
    else if (strncmp(item,"server_settings",BUFFER_SIZE) == 0)
    {
      GET_DATA("server_settings");
    }
  }
  else
  {
    settings = MHD_NO;
  } 
  return settings;

  #undef GET_DATA
}



/*
-----------------------------------------------------------------------------------------------------------
Name: post_request_completed
Description: Callback for when the post request is done
Parameters:
   cls - Callback value
   connection - The server connection
   settings - Settings
   request - Request status
Return: 0 if an error has occured, 1 if successfull
-----------------------------------------------------------------------------------------------------------
*/

void post_request_completed(void* cls, struct MHD_Connection* connection,void** settings, enum MHD_RequestTerminationCode request)
{
  // Variables
  struct connection_info_struct *connection_info = *settings;

  if (connection_info == NULL)
  {
    return;
  }
  if (connection_info->connection_type == 1)
  {
    MHD_destroy_post_processor(connection_info->post_result_settings);
  }

  free(connection_info);
  *settings = NULL;
}



/*
-----------------------------------------------------------------------------------------------------------
Name: get_statistics
Description: Gets the statistics from the database
Parameters:
   result - The json data for the statistics
Return: The http status
-----------------------------------------------------------------------------------------------------------
*/

int get_statistics(char* result)
{
  // Variables
  char* data;
  char* message = (char*)calloc(BUFFER_SIZE,sizeof(char));
  char* data2 = (char*)calloc(BUFFER_SIZE,sizeof(char));
  int count = 0;
  int counter = 0;
  long long int block_reward_number;
  long long int total_block_reward_number = 0;
  struct database_document_fields database_data;
  struct database_multiple_documents_fields database_multiple_documents_fields;
  int document_count = 0;  

  // define macros
  #define DATABASE_COLLECTION "statistics"
  #define DATA "{\"username\":\"XCASH\"}"

  #define pointer_reset_all \
  free(message); \
  message = NULL; \
  free(data2); \
  data2 = NULL;

  #define GET_STATISTICS_ERROR \
  memset(result,0,strnlen(result,BUFFER_SIZE)); \
  memcpy(result,"{\"Error\":\"Could not get the statistics\"}",40); \
  pointer_reset_database_array; \
  pointer_reset_all; \
  return 400;

  #define pointer_reset_database_array \
  for (count = 0; count < 10; count++) \
  { \
    pointer_reset(database_data.item[count]); \
    pointer_reset(database_data.value[count]); \
  } \
  for (count = 0; count < document_count; count++) \
  { \
    for (counter = 0; counter < 5; counter++) \
    { \
      pointer_reset(database_multiple_documents_fields.item[count][counter]); \
      pointer_reset(database_multiple_documents_fields.value[count][counter]); \
    } \
  }

  // check if the memory needed was allocated on the heap successfully
  if (message == NULL || data2 == NULL)
  {
    if (message != NULL)
    {
      pointer_reset(message);
    }
    if (data2 != NULL)
    {
      pointer_reset(data2);
    }
    GET_STATISTICS_ERROR
  };
  
  // initialize the database_document_fields struct 
  for (count = 0; count < 10; count++)
  {
    database_data.item[count] = (char*)calloc(100,sizeof(char));
    database_data.value[count] = (char*)calloc(100,sizeof(char));
  }
  
  document_count = count_all_documents_in_collection(DATABASE_NAME,"blocks_found",0);

  // initialize the database_multiple_documents_fields struct 
  for (count = 0; count < document_count; count++)
  {
    for (counter = 0; counter < 5; counter++)
    {
      database_multiple_documents_fields.item[count][counter] = (char*)calloc(RESERVE_PROOFS_LENGTH,sizeof(char));
      database_multiple_documents_fields.value[count][counter] = (char*)calloc(RESERVE_PROOFS_LENGTH,sizeof(char));
    }
  }
  database_multiple_documents_fields.document_count = 0;
  database_multiple_documents_fields.database_fields_count = 0;
  
  if (read_document_all_fields_from_collection(DATABASE_NAME,DATABASE_COLLECTION,DATA,&database_data,0) == 0)
  {
    pointer_reset_database_array;
    GET_STATISTICS_ERROR;
  }

  // add the total xcash to the database_document_fields struct  
  if (read_multiple_documents_all_fields_from_collection(DATABASE_NAME,"blocks_found","",&database_multiple_documents_fields,1,document_count,0,"",0,0) == 0)
  {
    GET_STATISTICS_ERROR;
  }

  for (count = 0; count < document_count; count++)
  {
    sscanf(database_multiple_documents_fields.value[count][3], "%lld", &block_reward_number);
    total_block_reward_number += block_reward_number;
  }

  memcpy(database_data.item[database_data.count],"total_xcash",11);
  sprintf(database_data.value[database_data.count],"%lld",total_block_reward_number);
  database_data.count++;  

  // add the total payments to the database_document_fields struct 
  total_block_reward_number = count_all_documents_in_collection(DATABASE_NAME,"public_addresses_payments",0); 
  memcpy(database_data.item[database_data.count],"total_payments",14);
  sprintf(database_data.value[database_data.count],"%lld",total_block_reward_number);
  database_data.count++;

  // add the total vote count to the database_document_fields struct
  memcpy(message,"{\"public_address_voted_for\":\"",29);
  memcpy(message+29,xcash_wallet_public_address,XCASH_WALLET_LENGTH);
  memcpy(message+127,"\"}",2);

  // check how many reserve proofs are for the public address
  counter = 0;
  for (count = 1; count <= 50; count++)
  { 
    memset(data2,0,strlen(data2));
    memcpy(data2,"reserve_proofs_",15);
    sprintf(data2+15,"%d",count);

    counter += count_documents_in_collection(DATABASE_NAME,data2,message,0);
  }

  memcpy(database_data.item[database_data.count],"total_vote_count",16);
  sprintf(database_data.value[database_data.count],"%d",counter);
  database_data.count++;

  // add the fee to the database_document_fields struct
  memcpy(database_data.item[database_data.count],"fee",3);
  sprintf(database_data.value[database_data.count],"%lf",fee);
  database_data.count++;

  // add the minimum amount to the database_document_fields struct
  memcpy(database_data.item[database_data.count],"minimum_amount",14);
  sprintf(database_data.value[database_data.count],"%lld",minimum_amount);
  database_data.count++;

  // add the public address to the database_document_fields struct
  memcpy(database_data.item[database_data.count],"public_address",14);
  memcpy(database_data.value[database_data.count],xcash_wallet_public_address,XCASH_WALLET_LENGTH);
  database_data.count++;

  // create a json string out of the database array of item and value
  if (create_json_data_from_database_document_array(&database_data,result,"username|") == 0)
  {
    pointer_reset_database_array;
    GET_STATISTICS_ERROR;
  }
 
  pointer_reset_database_array; 
  pointer_reset_all;
  return 200;

  #undef DATABASE_COLLECTION
  #undef DATA
  #undef pointer_reset_all
  #undef GET_STATISTICS_ERROR
  #undef pointer_reset_database_array
}



/*
-----------------------------------------------------------------------------------------------------------
Name: get_blocks_found
Description: Gets the blocks found
Parameters:
   result - The json data for the delegate voting information.
Return: The http status
-----------------------------------------------------------------------------------------------------------
*/

int get_blocks_found(char* result)
{
  // Variables
  char* data;
  char* data2 = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  char* data3 = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  char* message = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  int count = 0;
  int counter = 0;
  struct database_multiple_documents_fields database_data;
  int document_count = 0;

  // define macros
  #define DATABASE_COLLECTION "blocks_found"
  #define DATA "{\"username\":\"XCASH\"}"

  // define macros
  #define pointer_reset_all \
  free(data2); \
  data2 = NULL; \
  free(data3); \
  data3 = NULL; \
  free(message); \
  message = NULL;

  #define GET_BLOCKS_FOUND_ERROR \
  memset(result,0,strnlen(result,BUFFER_SIZE)); \
  memcpy(result,"{\"Error\":\"Could not get voters list data\"}",42); \
  pointer_reset_all; \
  return 400;
  
  #define pointer_reset_database_array \
  for (count = 0; count < document_count+1; count++) \
  { \
    for (counter = 0; counter < 5; counter++) \
    { \
      pointer_reset(database_data.item[count][counter]); \
      pointer_reset(database_data.value[count][counter]); \
    } \
  }

  // check if the memory needed was allocated on the heap successfully
  if (data2 == NULL || data3 == NULL || message == NULL)
  {
    if (data2 != NULL)
    {
      pointer_reset(data2);
    }
    if (data3 != NULL)
    {
      pointer_reset(data3);
    }
    if (message != NULL)
    {
      pointer_reset(message);
    }
    GET_BLOCKS_FOUND_ERROR;
  }

  // create the message
  memcpy(message,"{\"public_address_voted_for\":\"",29);
  memcpy(message+29,xcash_wallet_public_address,XCASH_WALLET_LENGTH);
  memcpy(message+127,"\"}",2);

  // get how many documents are in the collection
  document_count = count_all_documents_in_collection(DATABASE_NAME,DATABASE_COLLECTION,0);

  // initialize the database_multiple_documents_fields struct 
  for (count = 0; count < document_count+1; count++)
  {
    for (counter = 0; counter < 5; counter++)
    {
      database_data.item[count][counter] = (char*)calloc(RESERVE_PROOFS_LENGTH,sizeof(char));
      database_data.value[count][counter] = (char*)calloc(RESERVE_PROOFS_LENGTH,sizeof(char));
    }
  }
  database_data.document_count = 0;
  database_data.database_fields_count = 0;

  // get all of the blocks found
  if (read_multiple_documents_all_fields_from_collection(DATABASE_NAME,DATABASE_COLLECTION,"",&database_data,1,document_count,0,"",0,0) == 0)
  {
    GET_BLOCKS_FOUND_ERROR;
  }

  // add the total_blocks and total_blocks_found to the database_document_fields struct
  memset(data2,0,strlen(data2));
  memset(data3,0,strlen(data3));
  if (read_document_field_from_collection(DATABASE_NAME, "statistics", DATA, "total_blocks", data2, 0) == 0 || read_document_field_from_collection(DATABASE_NAME, "statistics", DATA, "total_blocks_found", data3, 0) == 0)
  {
    GET_BLOCKS_FOUND_ERROR;
  }
  memcpy(database_data.item[database_data.document_count][0],"total_blocks",12);
  memcpy(database_data.value[database_data.document_count][0],data2,strnlen(data2,BUFFER_SIZE));
  memcpy(database_data.item[database_data.document_count][1],"total_blocks_found",18);
  memcpy(database_data.value[database_data.document_count][1],data2,strnlen(data2,BUFFER_SIZE));
  
  database_data.document_count++;

  memset(result,0,strnlen(result,BUFFER_SIZE)); 
  if (create_json_data_from_database_multiple_documents_array(&database_data,result,"") == 0)
  {
    GET_BLOCKS_FOUND_ERROR;
  }  

  pointer_reset_database_array;
  pointer_reset_all;
  return 200;

  #undef DATABASE_COLLECTION
  #undef DATA
  #undef pointer_reset_all
  #undef GET_BLOCKS_FOUND_ERROR
  #undef pointer_reset_database_array
}



/*
-----------------------------------------------------------------------------------------------------------
Name: get_public_address_information
Description: Gets the statistics from the database
Parameters:
   connection - The server connection
   result - The json data for the statistics
Return: The http status
-----------------------------------------------------------------------------------------------------------
*/

int get_public_address_information(struct MHD_Connection* connection, char* result)
{
  // Variables
  char* data;
  char* message = (char*)calloc(BUFFER_SIZE,sizeof(char)); 
  int count = 0;
  struct database_document_fields database_data;

  // define macros
  #define DATABASE_COLLECTION "public_addresses"

  #define GET_PUBLIC_ADDRESS_INFORMATION_ERROR \
  memset(result,0,strnlen(result,BUFFER_SIZE)); \
  memcpy(result,"{\"Error\":\"Could not get public address information\"}",52); \
  pointer_reset(message); \
  return 400;

  #define pointer_reset_database_array \
  for (count = 0; count < 4; count++) \
  { \
    pointer_reset(database_data.item[count]); \
    pointer_reset(database_data.value[count]); \
  }
  
  // check if the memory needed was allocated on the heap successfully
  if (message == NULL)
  {
    GET_PUBLIC_ADDRESS_INFORMATION_ERROR;
  }

  // get the public_address
  if (MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "public_address") == NULL)
  {
    GET_PUBLIC_ADDRESS_INFORMATION_ERROR;
  } 
  data = (char*)MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "public_address"); 
  decode_html(data); 
  
  // error check
  if (strncmp(data,"",BUFFER_SIZE) == 0 || memcmp(data,XCASH_WALLET_PREFIX,3) != 0 || strnlen(data,BUFFER_SIZE) != XCASH_WALLET_LENGTH)
  { 
    GET_PUBLIC_ADDRESS_INFORMATION_ERROR;
  }

  // create the message
  memcpy(message,"{\"",2);
  memcpy(message+2,"public_address\":\"",17);
  memcpy(message+19,data,XCASH_WALLET_LENGTH);
  memcpy(message+19+XCASH_WALLET_LENGTH,"\"}",2); 
  
  // check if there is any data in the database that matches the message
  if (count_documents_in_collection(DATABASE_NAME,DATABASE_COLLECTION,message,0) <= 0)
  {
    GET_PUBLIC_ADDRESS_INFORMATION_ERROR;
  }
  
  // initialize the database_document_fields struct 
  for (count = 0; count < 4; count++)
  {
    database_data.item[count] = (char*)calloc(100,sizeof(char));
    database_data.value[count] = (char*)calloc(100,sizeof(char));
  }
  database_data.count = 0;
  
  if (read_document_all_fields_from_collection(DATABASE_NAME,DATABASE_COLLECTION,message,&database_data,0) == 0)
  {
    pointer_reset_database_array;
    GET_PUBLIC_ADDRESS_INFORMATION_ERROR;
  }

  // create a json string out of the database array of item and value
  if (create_json_data_from_database_document_array(&database_data,result,"") == 0)
  {
    pointer_reset_database_array;
    GET_PUBLIC_ADDRESS_INFORMATION_ERROR;
  }
 
  pointer_reset_database_array;  
  pointer_reset(message);  
  return 200;

  #undef DATABASE_COLLECTION
  #undef GET_PUBLIC_ADDRESS_INFORMATION_ERROR
  #undef pointer_reset_database_array
}



/*
-----------------------------------------------------------------------------------------------------------
Name: get_voters_list
Description: Gets the voters list
Parameters:
   result - The json data for the delegate voting information.
Return: The http status
-----------------------------------------------------------------------------------------------------------
*/

int get_voters_list(char* result)
{
  // Variables
  char* data;
  char* data2 = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  char* data3 = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  char* message = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  int count = 0;
  int counter = 0;
  struct database_multiple_documents_fields database_data;
  int document_count = 0;

  // define macros
  #define DATABASE_COLLECTION "public_addresses_payments"

  // define macros
  #define pointer_reset_all \
  free(data2); \
  data2 = NULL; \
  free(data3); \
  data3 = NULL; \
  free(message); \
  message = NULL;

  #define GET_VOTERS_LIST_ERROR \
  memset(result,0,strnlen(result,BUFFER_SIZE)); \
  memcpy(result,"{\"Error\":\"Could not get voters list data\"}",42); \
  pointer_reset_all; \
  return 400;
  
  #define pointer_reset_database_array \
  for (count = 0; count < document_count; count++) \
  { \
    for (counter = 0; counter < 4; counter++) \
    { \
      pointer_reset(database_data.item[count][counter]); \
      pointer_reset(database_data.value[count][counter]); \
    } \
  }

  // check if the memory needed was allocated on the heap successfully
  if (data2 == NULL || data3 == NULL || message == NULL)
  {
    if (data2 != NULL)
    {
      pointer_reset(data2);
    }
    if (data3 != NULL)
    {
      pointer_reset(data3);
    }
    if (message != NULL)
    {
      pointer_reset(message);
    }
    GET_VOTERS_LIST_ERROR;
  }

  // create the message
  memcpy(message,"{\"public_address_voted_for\":\"",29);
  memcpy(message+29,xcash_wallet_public_address,XCASH_WALLET_LENGTH);
  memcpy(message+127,"\"}",2);

  // check how many reserve proofs are for the public address
  for (count = 1; count <= 50; count++)
  { 
    memset(data2,0,strlen(data2));
    memcpy(data2,"reserve_proofs_",15);
    sprintf(data2+15,"%d",count);

    counter = count_documents_in_collection(DATABASE_NAME,data2,message,0);
    if (counter < 0)
    {
      GET_VOTERS_LIST_ERROR;
    }
    document_count += counter;
  }
  if (document_count <= 0)
  {
    GET_VOTERS_LIST_ERROR;
  }

  // initialize the database_multiple_documents_fields struct 
  for (count = 0; count < document_count; count++)
  {
    for (counter = 0; counter < 4; counter++)
    {
      database_data.item[count][counter] = (char*)calloc(RESERVE_PROOFS_LENGTH,sizeof(char));
      database_data.value[count][counter] = (char*)calloc(RESERVE_PROOFS_LENGTH,sizeof(char));
    }
  }
  database_data.document_count = 0;
  database_data.database_fields_count = 0;

  // create the message
  memset(data3,0,strnlen(data3,BUFFER_SIZE));
  memcpy(data3,"\"public_address_voted_for\" : \"",30);
  memcpy(data3+30,xcash_wallet_public_address,XCASH_WALLET_LENGTH);
  memcpy(data3+128,"\"",1);

  // get all of the reserve proofs for the public address
  for (count = 1; count <= 50; count++)
  { 
    memset(data2,0,strlen(data2));
    memcpy(data2,"reserve_proofs_",15);
    sprintf(data2+15,"%d",count);

    counter = count_documents_in_collection(DATABASE_NAME,data2,message,0);
    if (counter > 0)
    {
      if (read_multiple_documents_all_fields_from_collection(DATABASE_NAME,data2,data3,&database_data,1,counter,0,"",1,0) == 0)
      {
        GET_VOTERS_LIST_ERROR;
      }
    }
  }
  memset(result,0,strnlen(result,BUFFER_SIZE)); 
  if (create_json_data_from_database_multiple_documents_array(&database_data,result,"") == 0)
  {
    GET_VOTERS_LIST_ERROR;
  }  

  pointer_reset_database_array;
  pointer_reset_all;
  return 200;

  #undef DATABASE_COLLECTION
  #undef pointer_reset_all
  #undef GET_VOTERS_LIST_ERROR
  #undef pointer_reset_database_array
}



/*
-----------------------------------------------------------------------------------------------------------
Name: get_public_address_payment_information
Description: Gets the public address payment information from the database
Parameters:
   connection - The server connection
   result - The json data for the delegate voting information. Holds the parameter1 at the start
Return: The http status
-----------------------------------------------------------------------------------------------------------
*/

int get_public_address_payment_information(struct MHD_Connection* connection, char* result)
{
  // Variables
  char* data;
  char* message = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  int count = 0;
  int counter = 0;
  struct database_multiple_documents_fields database_data;
  int document_count = 0;

  // define macros
  #define DATABASE_COLLECTION "public_addresses_payments"
  #define GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR \
  memset(result,0,strnlen(result,BUFFER_SIZE)); \
  memcpy(result,"{\"Error\":\"Could not get public address information\"}",52); \
  pointer_reset(message); \
  return 400;
  
  #define pointer_reset_database_array \
  for (count = 0; count < document_count; count++) \
  { \
    for (counter = 0; counter < 5; counter++) \
    { \
      pointer_reset(database_data.item[count][counter]); \
      pointer_reset(database_data.value[count][counter]); \
    } \
  }

  
  // check if the memory needed was allocated on the heap successfully
  if (message == NULL)
  {
    GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR;
  }

  // get the public_address
  if (MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "public_address") == NULL)
  {
    GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR;
  } 
  data = (char*)MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "public_address"); 
  decode_html(data); 
  
  // error check
  if (strncmp(data,"",BUFFER_SIZE) == 0 || memcmp(data,XCASH_WALLET_PREFIX,3) != 0 || strnlen(data,BUFFER_SIZE) != XCASH_WALLET_LENGTH)
  {
    GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR;
  }

  // create the message
  memcpy(message,"{\"public_address\":\"",19);
  memcpy(message+19,data,XCASH_WALLET_LENGTH);
  memcpy(message+117,"\"}",2);

  // check if there is any data in the database that matches the message  
  document_count = count_documents_in_collection(DATABASE_NAME,"public_addresses_payments",message,0);
  if (document_count <= 0)
  {
    GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR;
  }
  
  // initialize the database_multiple_documents_fields struct 
  for (count = 0; count < document_count; count++)
  {
    for (counter = 0; counter < 5; counter++)
    {
      database_data.item[count][counter] = (char*)calloc(100,sizeof(char));
      database_data.value[count][counter] = (char*)calloc(100,sizeof(char));
    }
  }
  database_data.document_count = 0;
  database_data.database_fields_count = 0;

  // create the message
  memset(message,0,strnlen(message,BUFFER_SIZE));
  memcpy(message,"\"public_address\" : \"",20);
  memcpy(message+20,data,XCASH_WALLET_LENGTH);
  memcpy(message+20+XCASH_WALLET_LENGTH,"\"",1);

  if (read_multiple_documents_all_fields_from_collection(DATABASE_NAME,DATABASE_COLLECTION,message,&database_data,1,document_count,0,"",0,0) == 0)
  {
    GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR;
  }

  memset(result,0,strnlen(result,BUFFER_SIZE)); 
  if (create_json_data_from_database_multiple_documents_array(&database_data,result,"") == 0)
  {
    GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR;
  }

  pointer_reset_database_array;
  pointer_reset(message);
  return 200;

  #undef DATABASE_COLLECTION
  #undef GET_PUBLIC_ADDRESS_PAYMENT_INFORMATION_ERROR
  #undef pointer_reset_database_array
}



/*
-----------------------------------------------------------------------------------------------------------
Name: create_server
Description: Creates the server
Parameters:
   cls - Callback value
   connection -Server connection
   url - The url
   method - The HTTP method
   version - The HTPP version
   upload_data - The HTTP data
   upload_data_size - The size of the HTTP Data
   settings - Settings
Return: 0 if an error has occured, 1 if successfull
-----------------------------------------------------------------------------------------------------------
*/

int create_server(void* cls,struct MHD_Connection* connection,const char* url,const char* method,const char* version,const char* upload_data,size_t* upload_data_size,void** settings)
{
  // Constants
  const char* errorstr = "<html><body>An internal server error has occured!</body></html>";

  // Variables 
  char* data = (char*)calloc(BUFFER_SIZE,sizeof(char));  
  int server;
  int http_status;
  struct MHD_Response* response;
  size_t count = 0;
  FILE* file;
  
  #define pointer_reset_all \
  free(data); \
  data = NULL; 
  
  #define HTTP_RESULTS(http_data,content_type,http_status) \
  response = MHD_create_response_from_buffer(strnlen(http_data,BUFFER_SIZE),(void *)http_data,MHD_RESPMEM_MUST_FREE); \
  MHD_add_response_header(response, "Access-Control-Allow-Origin", "*"); \
  MHD_add_response_header(response, "Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept"); \
  MHD_add_response_header(response, "Content-Type", content_type); \
  MHD_queue_response(connection, http_status, response); \
  MHD_destroy_response(response); \
  return MHD_YES;

  if (data == NULL)
  {
    return 0;
  }

  // error check
  if (strncmp(method, MHD_HTTP_METHOD_GET,BUFFER_SIZE) != 0 && strncmp(method, MHD_HTTP_METHOD_POST,BUFFER_SIZE) != 0)
  {
    pointer_reset_all;
    return MHD_NO;
  }

   if (*settings == NULL)
   {
     struct connection_info_struct* connection_info = malloc(sizeof(struct connection_info_struct));
     connection_info->count = 0;

     if (strncmp(method,MHD_HTTP_METHOD_POST,BUFFER_SIZE) == 0)
     {
       connection_info->post_result_settings = MHD_create_post_processor(connection, READ_FILE_BUFFER_SIZE,post_result,(void*)connection_info);
       if (connection_info->post_result_settings == NULL)
       {
         pointer_reset(connection_info);
         pointer_reset_all;
         return MHD_NO;
       }
       else
       {
         connection_info->connection_type = 1;
         *settings = (void *) connection_info;
         pointer_reset_all;
         return MHD_YES;
       }       
      }
   }

   if (strncmp(method,MHD_HTTP_METHOD_POST,BUFFER_SIZE) == 0)
   {
     struct connection_info_struct* connection_info = *settings;     

     if (*upload_data_size != 0)
     {
       MHD_post_process(connection_info->post_result_settings, upload_data,*upload_data_size);
       *upload_data_size = 0;
       pointer_reset_all;
       return MHD_YES;
     }
   }


   if (strncmp(method,MHD_HTTP_METHOD_GET,BUFFER_SIZE) == 0)
   {
     if (strncmp(url,"/getstatistics",BUFFER_SIZE) == 0)
     {
       memset(data,0,strnlen(data,BUFFER_SIZE));
       http_status = get_statistics(data);
       HTTP_RESULTS(data,"application/json",http_status);
     }
     if (strncmp(url,"/getblocksfound",BUFFER_SIZE) == 0)
     {
       memset(data,0,strnlen(data,BUFFER_SIZE));
       http_status = get_blocks_found(data);
       HTTP_RESULTS(data,"application/json",http_status);
     }
     else if (strncmp(url,"/getpublicaddressinformation",BUFFER_SIZE) == 0)
     { 
       memset(data,0,strnlen(data,BUFFER_SIZE));  
       http_status = get_public_address_information(connection,data);
       HTTP_RESULTS(data,"application/json",http_status);
     }     
     else if (strncmp(url,"/getvoterslist",BUFFER_SIZE) == 0)
     { 
       memset(data,0,strnlen(data,BUFFER_SIZE));  
       http_status = get_voters_list(data);
       HTTP_RESULTS(data,"application/json",http_status);
     }         
     else if (strncmp(url,"/getpublicaddresspaymentinformation",BUFFER_SIZE) == 0)
     { 
       memset(data,0,strnlen(data,BUFFER_SIZE));  
       http_status = get_public_address_payment_information(connection,data);
       HTTP_RESULTS(data,"application/json",http_status);
     }      
     else
     {
       // check if the file exist
       file = fopen(&url[1],"rb");
       if (file == NULL)
       {
         // open the index.html page
         file = fopen("index.html","rb");
       } 
       // get the file size
       fseek(file, 0, SEEK_END);
       const long file_size = ftell(file);
       fseek(file, 0, SEEK_SET); 

       response = MHD_create_response_from_callback(file_size, READ_FILE_BUFFER_SIZE,&read_file_server,file,&callback_reset);
       if (response == NULL)
       {
  	     fclose(file);
         pointer_reset_all;
     	   return MHD_NO;
     	 }
       // set the content type
       if (strstr(&url[1],".html") != NULL)
       {
         MHD_add_response_header(response, "Content-Type", "text/html");
       }
       if (strstr(&url[1],".js") != NULL)
       {
         MHD_add_response_header(response, "Content-Type", "application/javascript");
       }
       if (strstr(&url[1],".css") != NULL)
       {
         MHD_add_response_header(response, "Content-Type", "text/css");
       }
       if (strstr(&url[1],".png") != NULL)
       {
         MHD_add_response_header(response, "Content-Type", "image/png");
       }
       if (strstr(&url[1],".jpg") != NULL || strstr(&url[1],".jpeg") != NULL)
       {
         MHD_add_response_header(response, "Content-Type", "image/jpeg");
       }
       MHD_queue_response(connection, MHD_HTTP_OK, response);
       MHD_destroy_response(response);
       pointer_reset_all;
       return MHD_YES;
     }  
   }

  #undef pointer_reset_all   
  #undef HTTP_RESULTS
}