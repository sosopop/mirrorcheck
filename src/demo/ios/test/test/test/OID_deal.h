#include<stdio.h>
//////kind of code 
typedef enum{
 general_code=0,
 book_code,
 laguage_code,
 game_code,
 voice_add_code,
 voice_reduce_code,
 re_read_stop_code,
 stop_code,
 music_code,
 story_code,
 ancient_CH_code,
 dictionary_code,
 calcul_code,
 other_code
}OID_type;

//////code from the company
typedef enum{
 ZX_series=0,
 QX_series,
}F_company;
//////code of features
typedef struct{
 int type_code;
 F_company series_code;
}code_features;

/*
  describetion function: deal_main
  input:    OID_Code            this is a data collected by the pen .
  input:    before_series       code from the company,init date or before used date.
  output:   code_features       this code feature,inlude type_code and code from the company.
*/
code_features deal_main(int OID_Code,int before_series);

