
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

//#define PRODUCE_SQL
#define PRODUCE_B

FILE * pFile = NULL;
long lSize;
char * buffer = NULL;
size_t result;

#ifdef PRODUCE_SQL
FILE * oFile = NULL;
#endif
#ifdef PRODUCE_B
FILE * bFile = NULL;
#endif

const char * files[] = {
    "030/m/030m11/030m11_0200_deme",
    "030/m/030m12/030m12_0200_deme",
    "030/m/030m13/030m13_0200_deme",
    "030/m/030m14/030m14_0200_deme",
    "030/m/030m11/030m11_0200_demw",
    "030/m/030m12/030m12_0200_demw",
    "030/m/030m13/030m13_0200_demw",
    "030/m/030m14/030m14_0200_demw",
  };

void get_chars(int count)
{
  buffer = (char*)realloc(buffer, count+1);
  
  result = fread(buffer, 1, count, pFile);
  buffer[count] = '\0';
}

int main()
{
  for (int file_id = 0 ; file_id < sizeof(files)/sizeof(char*) ; file_id++)
  {
    char fn[150];
    sprintf(fn, "%s.dem", files[file_id]); 
    printf("reading from '%s'\n", fn);
    pFile = fopen(fn, "rb");
    if (pFile==NULL) { fputs("File error", stderr); exit(1); }
    
#ifdef PRODUCE_SQL
    sprintf(fn, "%s.sql", files[file_id]);
    printf("writing to '%s'\n", fn);
    oFile = fopen(fn, "w");
    if (oFile==NULL) { fputs("File error", stderr); exit(1); }
#endif

#ifdef PRODUCE_B
    sprintf(fn, "%s.b", files[file_id]);
    printf("writing to '%s'\n", fn);
    bFile = fopen(fn, "w");
    if (bFile==NULL) { fputs("File error", stderr); exit(1); }
#endif
    
    //fprintf(oFile, "DROP TABLE IF EXISTS vertexes;\n");
    //fprintf(oFile, "CREATE TABLE vertexes (id INT PRIMARY KEY AUTO_INCREMENT, lat FLOAT(15,10), lng FLOAT(15,10), alt INT);\n");
    
    fseek (pFile, 0, SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
    
    get_chars(40+60+9); // junk
    get_chars(4); int lng_deg = atoi(buffer);
    get_chars(2); int lng_min = atoi(buffer);
    get_chars(7); float lng_sec = atof(buffer);
    get_chars(4); int lat_deg = atoi(buffer);
    get_chars(2); int lat_min = atoi(buffer);
    get_chars(7); float lat_sec = atof(buffer);
    
    float lng = (fabs(lng_deg) + lng_min/60.0 + lng_sec/3600.0) * (lng_deg < 0 ? -1 : 1);
    float lat = (fabs(lat_deg) + lat_min/60.0 + lat_sec/3600.0) * (lng_deg < 0 ? -1 : 1);
    //printf("%d %d %f (%f)\n", lng_deg, lng_min, lng_sec, lng);
    //printf("%d %d %f (%f)\n", lat_deg, lat_min, lat_sec, lat);
    
    get_chars(1+1+3+4+6+6+6+6+15*24+6+6+6); // junk
    
    get_chars(24); float sw_lng = atof(buffer) / 3600.00;
    get_chars(24); float sw_lat = atof(buffer) / 3600.00;
    get_chars(24); float nw_lng = atof(buffer) / 3600.00;
    get_chars(24); float nw_lat = atof(buffer) / 3600.00;
    get_chars(24); float ne_lng = atof(buffer) / 3600.00;
    get_chars(24); float ne_lat = atof(buffer) / 3600.00;
    get_chars(24); float se_lng = atof(buffer) / 3600.00;
    get_chars(24); float se_lat = atof(buffer) / 3600.00;
    //printf("%f %f\n", sw_lng, sw_lat);
    //printf("%f %f\n", nw_lng, nw_lat);
    //printf("%f %f\n", ne_lng, ne_lat);
    //printf("%f %f\n", se_lng, se_lat);

#ifdef PRODUCE_B
    short dimension = 1201;
    fwrite(&dimension, sizeof(short), 1, bFile); // x
    fwrite(&dimension, sizeof(short), 1, bFile); // y
    
    fwrite(&nw_lat, sizeof(float), 1, bFile);
    fwrite(&nw_lng, sizeof(float), 1, bFile);
    fwrite(&se_lat, sizeof(float), 1, bFile);
    fwrite(&se_lng, sizeof(float), 1, bFile);
#endif
    
    get_chars(24); float min_elev = atof(buffer);
    get_chars(24); float max_elev = atof(buffer);
    //printf("%f %f\n", min_elev, max_elev);  // min/max
    
    get_chars(24+6);
    get_chars(12); float x_res = atof(buffer);
    get_chars(12); float y_res = atof(buffer);
    get_chars(12); float z_res = atof(buffer);
    //printf("%f %f %f\n", x_res, y_res, z_res);
    //  +2*6+5+
    //  1+5+1+4+4+1+1+2+
    //  2+2+4+4+4*2+7); // junk
    
    rewind(pFile);
    
    get_chars(1024); // Record type A
    
    int type_b_header_size = 2*6 + 2*6 + 2*24 + 24 + 2*24;
    int type_b_data_size = ceil((6*1201+type_b_header_size)/1024.0)*1024 - type_b_header_size;
    
    for (int col_id = 0 ; col_id < 1201 ; col_id++)
    {
      get_chars(6);
      int row = atoi(buffer);
      assert(row == 1);
      get_chars(6);
      int col = atoi(buffer);
      
      get_chars(6);
      int rn = atoi(buffer); // 1201 for canada
      assert(rn == 1201);
      get_chars(6);
      int n = atoi(buffer); // 1
      assert(n == 1);
      
      get_chars(24);
      float lng = atof(buffer) / 3600.00; // long in arc seconds
      get_chars(24);
      float lat = atof(buffer) / 3600.00; // lat in arc seconds
      //printf("%d %f %f\n", col, lng, lat);
      
      get_chars(24); // always 0.0
      
      get_chars(24);
      float min = atof(buffer); // min elevation
      get_chars(24);
      float max = atof(buffer); // max elevation
      
      int elevation_data[1201];
      
      int char_count = 0;
      int row_id = 0;
      while (row_id < 1201)
      {
        get_chars(6); //short alt = atoi(buffer);
        char_count += 6;
        elevation_data[row_id] = atoi(buffer);
        
        if (elevation_data[row_id] == -32767) elevation_data[row_id] = 75; // kbfu, applies to toronto data only, USA is invalid but it's all water anyway
        
#ifdef PRODUCE_SQL
        if (row_id == 0) fprintf(oFile, "INSERT INTO vertexes (lat,lng,alt) VALUES (\n");
        else fprintf(oFile, "),%s(", row_id%5==0?"\n":"");
        fprintf(oFile, "%f,%f,%d", lat+(row_id*y_res/3600.00), lng, elevation_data[row_id]);
#endif
        
#ifdef PRODUCE_B
        fwrite(&elevation_data[row_id], sizeof(short), 1, bFile);
#endif
        
        row_id ++;
        if (((row_id - 146) % 170 == 0 && row_id != 0) || row_id == 146) { get_chars(4); char_count += 4; } // junk
      }
      if (type_b_data_size > char_count)
        get_chars(type_b_data_size - char_count); // junk
      
#ifdef PRODUCE_SQL
      fprintf(oFile, ");\n");
#endif
    }
    //get_chars(type_b_header_size);
    //printf("%s\n", buffer);
    //get_chars(ceil((6*1201+type_b_header_size)/1024.0)*1024 - type_b_header_size);
    //printf("%s------------\n", buffer);
    
    //char temp[100];
    //sprintf(temp, "cp %s 030/", fn);
    //printf("%s\n", temp);
    //system(temp);

    fclose(pFile);

#ifdef PRODUCE_SQL
    fclose(oFile);
#endif
#ifdef PRODUCE_B
    fclose(bFile);
#endif
  }
  
  free(buffer);
}