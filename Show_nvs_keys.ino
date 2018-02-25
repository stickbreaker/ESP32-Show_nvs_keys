// Show_nvs_keys.ino
// Read all the keys from nvs partition and dump this information.
// 04-dec-2017, Ed Smallenburg.
// 25-FEB-2018, @stickbreaker
//
#include <stdio.h>
#include <string.h>
#include <esp_partition.h>
// Debug buffer size

/* from nvs_types.hpp
enum class ItemType : uint8_t {
    U8   = 0x01,
    I8   = 0x11,
    U16  = 0x02,
    I16  = 0x12,
    U32  = 0x04,
    I32  = 0x14,
    U64  = 0x08,
    I64  = 0x18,
    SZ   = 0x21,
    BLOB = 0x41,
    ANY  = 0xff
};

*/

// From IDF nvs_page.hpp
 const uint32_t PSB_INIT = 0x1;
 const uint32_t PSB_FULL = 0x2;
 const uint32_t PSB_FREEING = 0x4;
 const uint32_t PSB_CORRUPT = 0x8;

 const uint32_t ESB_WRITTEN = 0x1;
 const uint32_t ESB_ERASED = 0x2;

 const uint32_t SEC_SIZE = SPI_FLASH_SEC_SIZE;

 const size_t ENTRY_SIZE  = 32;
 const size_t ENTRY_COUNT = 126;
 const uint32_t INVALID_ENTRY = 0xffffffff;
    
 const size_t BLOB_MAX_SIZE = ENTRY_SIZE * (ENTRY_COUNT / 2 - 1);

 const uint8_t NS_INDEX = 0;
 const uint8_t NS_ANY = 255;

 
        // All bits set, default state after flash erase. Page has not been initialized yet.
 const uint32_t UNINITIALIZED = 0xffffffff;

        // Page is initialized, and will accept writes.
 const uint32_t ACTIVE        = UNINITIALIZED & ~PSB_INIT;

        // Page is marked as full and will not accept new writes.
 const uint32_t FULL          = ACTIVE & ~PSB_FULL;

        // Data is being moved from this page to a new one.
const uint32_t  FREEING       = FULL & ~PSB_FREEING;

        // Page was found to be in a corrupt and unrecoverable state.
        // Instead of being erased immediately, it will be kept for diagnostics and data recovery.
        // It will be erased once we run out out free pages.
const uint32_t  CORRUPT       = FREEING & ~PSB_CORRUPT;

        // Page object wasn't loaded from flash memory
const uint32_t  INVALID       = 0;

// end from nvs_page.hpp

struct NVS_DATA{
  union{
    struct {
      uint16_t size;
      uint16_t rsv;
      uint32_t CRC;
      };
    struct {
      uint8_t data_u8;
      uint8_t f0[7];
      };
    struct {
      int8_t data_i8;
      uint8_t f1[7];
      };
    struct {
      uint16_t data_u16;
      uint8_t f2[6];
      };
    struct {
      int16_t data_i16;
      uint8_t f3[6];
      };
    struct {
      uint32_t data_u32;
      uint8_t f4[4];
      };
    struct {
      int32_t data_i32;
      uint8_t f5[4];
      };
    struct {
      uint64_t data_u64;
      };
    struct {
      int64_t data_i64;
      };
    };
  };
  
struct NVS_ENTRY {
  uint8_t  Ns;         // Namespace ID
  uint8_t  Type;       // Type of value
  uint8_t  Span;       // Number of entries used for this item
  uint8_t  Rvs;        // Reserved, should be 0xFF
  uint32_t CRC;        // CRC
  char     Key[16];    // Key in Ascii
  NVS_DATA Data;       // Data in entry 
  };

struct nvs_page {       // For nvs entries 1 page is 4096
  uint32_t  State;     
  uint32_t  Seqnr;
  uint32_t  Unused[5];
  uint32_t  CRC;
  uint8_t   Bitmap[32];
  NVS_ENTRY Entry[126];
  };

struct NAME_SPACE {
  uint8_t nsID;
  char    name[16];
  };
  
struct NS_ARRAY{
  uint8_t count;
  NAME_SPACE ns[255];
};
  
  
// Common data
nvs_page buf;
uint8_t * pageOrder=NULL; // current sequence for nvs pages pageOrder[0] is first page,
  // usage is: offset = pageOrder[page]*sizeof(nvs_page);
NS_ARRAY * NA=NULL;  

// 
// dispPageState, interpret the nvs Page State bits 
//  
void dispPageState(uint32_t state){
if((state & ~PSB_INIT)==state) Serial.print("INIT ");
if((state & ~PSB_FULL)==state) Serial.print("FULL ");
if((state & ~PSB_FREEING)==state) Serial.print("FREEING ");
if((state & ~PSB_CORRUPT)==state) Serial.print("CORRUPT ");
Serial.print(" means ");
if((state == UNINITIALIZED)) Serial.print("UNINITIALIZED ");
if((state == ACTIVE )) Serial.print("ACTIVE ");
if((state == FULL)) Serial.print("FULL ");
if((state == FREEING)) Serial.print("FREEING ");
if((state == CORRUPT)) Serial.print("CORRUPT ");
if((state == INVALID)) Serial.print("INVALID ");

}

//
// initialize logical page translation table, NameSpace list.
//
void refreshNVS(const esp_partition_t* nvs ){
if(NA) free(NA);
NA =(NS_ARRAY*)malloc(1);
NA->count=0;
if(pageOrder) free(pageOrder);
uint8_t i = 0;
uint16_t offset = 0;
esp_err_t result = ESP_OK;
i = nvs->size / sizeof(nvs_page);
pageOrder = (uint8_t*)malloc(i);
while(i>0) pageOrder[--i]=255;
i=0;
offset=0;
while(i< (nvs->size / sizeof(nvs_page))){
  offset = i*sizeof(nvs_page);
  result = esp_partition_read( nvs, offset, &buf, sizeof(nvs_page));  // 
  if(result == ESP_OK){
    Serial.printf("page[%u].State=0x%lx ",i,buf.State);
    dispPageState(buf.State);
    if((buf.State == ACTIVE)||(buf.State==FULL)){
      if(buf.Seqnr>((nvs->size / sizeof(nvs_page))-1)){
        Serial.printf("Sequence value InValid: page[%u].Seqnr=%u ",i,buf.Seqnr);
        }
      else {
        if(pageOrder[buf.Seqnr]!=255){
          Serial.printf("Sequence error %u = %u, pageOrder[%u]=%u Already inuse!",i,buf.Seqnr,buf.Seqnr,pageOrder[
           buf.Seqnr]);
          }
        else {
          pageOrder[buf.Seqnr] = i;
          Serial.printf(" Seqnr=%u ",buf.Seqnr);
          }
        }
      }
    else Serial.printf(" Page[%u] not active, State = 0x%lx Seqnr=%u ",i,buf.State,buf.Seqnr);
    }
  else Serial.printf(" Page[%u] read Error = 0x%lx",i,result);
  Serial.println();
  i++;
  }  

/* manually create pageOrder because My ESP32 NVS is hosed
pageOrder[1]=1; // jam  
pageOrder[2]=3;
pageOrder[3]=4;
*/
Serial.printf("\nLogical Page to Physical Sector\n");
for(i=0;i<(nvs->size / sizeof(nvs_page));i++){
  Serial.printf("Page %d -> %u\n",i,pageOrder[i]);
  }
// find namespaces
i=0;
while(i<(nvs->size / sizeof(nvs_page))){
  if(pageOrder[i] != 255) {
    offset = pageOrder[i] * sizeof(nvs_page);
    result = esp_partition_read( nvs, offset, &buf, sizeof(nvs_page));  // 
    if(result == ESP_OK){
      uint8_t j=0,bm;
      while(j<126){
        bm =(buf.Bitmap[j/4] >>((j%4)*2)) & 0x03;  // Get bitmap for this entry
        if((bm == 2) &&(buf.Entry[j].Ns == 0)){ // found namespace entry
        // add to NA
          NA = (NS_ARRAY*)realloc(NA,(((++NA->count)*sizeof(NAME_SPACE))+1));
          NA->ns[NA->count-1].nsID= buf.Entry[j].Data.data_u8;
          strcpy(NA->ns[NA->count-1].name, buf.Entry[j].Key);
          }
        if(bm==2) {
          j += buf.Entry[j].Span;                             // Next entry
          }
        else {
          j++;
          }
        }
      }
    }
  i++;      
  }    
Serial.printf("\nNAME SPACES\n");
for(i=0;i<NA->count;i++){
  Serial.printf("%15s = %u\n",NA->ns[i].name,NA->ns[i].nsID);  
  }

}

//
// print Selected nvs entry from the currently loaded nvs page
//
uint8_t disp_nvs_data( uint8_t index){ // returns next index
char * desc, *d1;
uint16_t descSize = 0,i=0,j=0,n=0;
char dtype[5];
char dvalue[32];
NVS_DATA * ndata;
NVS_ENTRY * nvs;

nvs = &buf.Entry[index];
uint8_t inc = nvs->Span;
switch(nvs->Type){
  case 0x01 : // U8
    sprintf(dtype,"U8  ");
    sprintf(dvalue,"%u",nvs->Data.data_u8);
    desc =(char*)&dvalue;
    break;
  case 0x02 : // U16
    sprintf(dtype,"U16 ");
    sprintf(dvalue,"%u",nvs->Data.data_u16);
    desc =(char*)&dvalue;
    break;
  case 0x04 : // U32
    sprintf(dtype,"U32 ");
    sprintf(dvalue,"%lu",nvs->Data.data_u32);
    desc =(char*)&dvalue;
    break;
  case 0x08 : // U64
    sprintf(dtype,"U64 ");
    sprintf(dvalue,"%llu",nvs->Data.data_u64);
    desc =(char*)&dvalue;
    break;
  case 0x11 : // i8
    sprintf(dtype,"I8  ");
    sprintf(dvalue,"%u",nvs->Data.data_u8);
    desc =(char*)&dvalue;
    break;
  case 0x12 : // i16
    sprintf(dtype,"I16 ");
    sprintf(dvalue,"%d",nvs->Data.data_i16);
    desc =(char*)&dvalue;
    break;
  case 0x14 : // i32
    sprintf(dtype,"I32 ");
    sprintf(dvalue,"%ld",nvs->Data.data_i32);
    desc =(char*)&dvalue;
    break;
  case 0x18 : // i64
    sprintf(dtype,"I64 ");
    sprintf(dvalue,"%lld",nvs->Data.data_i64);
    desc =(char*)&dvalue;
    break;
  case 0x21: // SZ, String Zero terminated
    sprintf(dtype,"SZ  ");
    desc = (char*)&buf.Entry[index+1];
    break;
  case 0x41: // BLOB
    sprintf(dtype,"BLOB");
    
    descSize = (nvs->Data.size/16);
    if((nvs->Data.size%16)!=0) descSize++;
    descSize = (descSize *(2+16*4))+1+11;
    desc = (char*)malloc(descSize);
    memset(desc,32,descSize);
    desc[descSize-1] = '\0';
 //   Serial.printf("Blob size=%d calc=%d\n",nvs->Data.size,descSize);
    d1 = (char*)&buf.Entry[index+1];
    i=sprintf(desc,"size:%04x\n",nvs->Data.size);
    j=0;
    n=0;
    while(j< nvs->Data.size){
      desc[i+((16-n)*3)+n+1] = (d1[j]>31)&&(d1[j]<128)?d1[j]:'.';
      i+= sprintf((char*)&desc[i],"%02x ",d1[j]);
      n++;
      j++;
      if(n>15){
        desc[i]=' ';
        desc[i+17]='\n';
        i=i+18;
        desc[i] = '\0';
        n=0;
        }
      }
    if(n>0){// partial line
      desc[i]=' ';
      i = i+17+(16-n)*3;
      desc[i] = '\0';
      }   
    else { // complete line
      desc[i-1] = '\0';
      }    
 break;

  default :
    sprintf(dtype,"%03u",nvs->Type);
    sprintf(dvalue,"%016x",nvs->Data.data_u64);
    desc= (char*)&dvalue;
    inc = 1;
  }
Serial.printf("%03d:%15s type:%s data=%s\n",index,nvs->Key,dtype,desc);
if(descSize) free(desc);
return index + inc;  
}

//
// listKeys for a specified namespace
//
void listKeys(const esp_partition_t *nvs, uint8_t id){
uint8_t i = 0;
uint16_t offset=0;
esp_err_t result=ESP_OK;

i=0;
while(i<(nvs->size / sizeof(nvs_page))){
  if(pageOrder[i] != 255) {
    offset = pageOrder[i] * sizeof(nvs_page);
    result = esp_partition_read( nvs, offset, &buf, sizeof(nvs_page));  // 
    if(result == ESP_OK){
      uint8_t j=0,bm;
      while(j<126){
        bm =(buf.Bitmap[j/4] >>((j%4)*2)) & 0x03;  // Get bitmap for this entry
        if((bm == 2) &&(buf.Entry[j].Ns == id)){ // found namespace entry
          Serial.printf("%u-",i);
          disp_nvs_data(j);
          }
        if(bm==2) {
          j += buf.Entry[j].Span;                             // Next entry
          }
        else {
          j++;
          }
        }
      }
    }
  i++;      
  }    
}
  
//**************************************************************************************************
//                                          S E T U P                                              *
//**************************************************************************************************
//**************************************************************************************************
void setup()
{
  esp_partition_iterator_t  pi;                              // Iterator for find
  const esp_partition_t*    nvs;                             // Pointer to partition struct
  esp_err_t                 result = ESP_OK;
  const char*               partname = "nvs";
  uint8_t                   i;                               // Index in Entry 0..125
  uint8_t                   bm;                              // Bitmap for an entry
  
  Serial.begin(115200);                                   // For debug
  Serial.println();
  pi = esp_partition_find( ESP_PARTITION_TYPE_DATA,          // Get partition iterator for
                            ESP_PARTITION_SUBTYPE_ANY,        // this partition
                            partname );
  if(pi) {
    nvs = esp_partition_get( pi );                          // Get partition struct
    esp_partition_iterator_release( pi );                   // Release the iterator
    Serial.printf( "Partition %s found, %d bytes\n", partname, nvs->size );
    }
  else {
    Serial.printf( "Partition %s not found!\n", partname );
    return;
    }

  refreshNVS(nvs);  // initialize pageOrder[] and NA

  i=0;
  while(i<NA->count){
    Serial.printf("\nNameSpace: %15s = %u\n",NA->ns[i].name,NA->ns[i].nsID);
    i++;
    }
 
 i=0;
 uint8_t j=0;
  while(i<255){
    if(i<NA->count){
      Serial.printf("\nNameSpace: %15s \n",NA->ns[i].name);
      j=NA->ns[i].nsID;
      }
    else {
      j = i+1;
      Serial.printf("\nOrphan UnNamed : %u \n",j);
      }
    listKeys(nvs,j);
    i++;
    }
    
}

void loop() {
  // put your main code here, to run repeatedly:
}
