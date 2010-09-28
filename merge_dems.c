
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

const char * file_merges[] = {  // WEST, EAST, DEST   or NORTH, SOUTH, DEST
  //"030/m/030m11/030m11_0200_demw.b", "030/m/030m11/030m11_0200_deme.b", "030/m/030m11/030m11_0200_dem.b",
  //"030/m/030m12/030m12_0200_demw.b", "030/m/030m12/030m12_0200_deme.b", "030/m/030m12/030m12_0200_dem.b",
  //"030/m/030m13/030m13_0200_demw.b", "030/m/030m13/030m13_0200_deme.b", "030/m/030m13/030m13_0200_dem.b",
  //"030/m/030m14/030m14_0200_demw.b", "030/m/030m14/030m14_0200_deme.b", "030/m/030m14/030m14_0200_dem.b",
  
  //"030/m/030m12/030m12_0200_dem.b", "030/m/030m11/030m11_0200_dem.b", "030/m/030m12-11_0200_dem.b",
  //"030/m/030m13/030m13_0200_dem.b", "030/m/030m14/030m14_0200_dem.b", "030/m/030m13-14_0200_dem.b",
  "030/m/030m12-11_0200_dem.b", "030/m/030m13-14_0200_dem.b", "030/m/030m12-11-13-14_0200_dem.b",
}; 

int main()
{
  int num_file_merges = sizeof(file_merges) / sizeof(const char *) / 3;
  printf("num_file_merges = %d (%d/%d/3)\n", num_file_merges, (int)sizeof(file_merges), (int)sizeof(const char *), 3);
  
  long file_size_1;
  short size_1[2];
  float bbox_1[4];
  short * data_1 = NULL;
  
  long file_size_2;
  short size_2[2];
  float bbox_2[4];
  short * data_2 = NULL;
  
  short size_3[2];
  float bbox_3[4];
  short * data_3 = NULL;
  
  int merge_id;
  for (merge_id = 0 ; merge_id < num_file_merges ; merge_id++)
  {
    FILE * fp_1 = fopen(file_merges[merge_id*3], "rb");
    if (fp_1 == NULL) { printf("could not open '%s'\n", file_merges[merge_id*3]); exit(1); }
    FILE * fp_2 = fopen(file_merges[merge_id*3+1], "rb");
    if (fp_2 == NULL) { printf("could not open '%s'\n", file_merges[merge_id*3+1]); exit(1); }
    FILE * fp_3 = fopen(file_merges[merge_id*3+2], "wb");
    if (fp_3 == NULL) { printf("could not open '%s'\n", file_merges[merge_id*3+2]); exit(1); }
    
    fseek (fp_1, 0, SEEK_END);
    file_size_1 = ftell (fp_1);
    rewind (fp_1);
    if (file_size_1 < 2*sizeof(short)+4*sizeof(float)) { printf("file %s not large enough (%ld)\n", file_merges[merge_id*3], file_size_1); exit(1); }
    if (file_size_2 < 2*sizeof(short)+4*sizeof(float)) { printf("file %s not large enough (%ld)\n", file_merges[merge_id*3+1], file_size_2); exit(1); }
    
    fseek (fp_2, 0, SEEK_END);
    file_size_2 = ftell (fp_2);
    rewind (fp_2);
    
    // load files 1 & 2
    {
      fread(&size_1, 2, sizeof(short), fp_1);  // x, y
      fread(&bbox_1, 4, sizeof(float), fp_1);  // nw(lat, lng) -> se(lat, lng)
      if (file_size_1 < 2*sizeof(short)+4*sizeof(float)+sizeof(short)*size_1[0]*size_1[1]) { printf("file %s not large enough (%ld)\n", file_merges[merge_id*3], file_size_1); exit(1); }
      data_1 = (short*)realloc(data_1, sizeof(short)*size_1[0]*size_1[1]);
      fread(data_1, size_1[0]*size_1[1], sizeof(short), fp_1);
      
      fread(&size_2, 2, sizeof(short), fp_2);  // x, y
      fread(&bbox_2, 4, sizeof(float), fp_2);  // nw(lat, lng) -> se(lat, lng)
      if (file_size_2 < 2*sizeof(short)+4*sizeof(float)+sizeof(short)*size_2[0]*size_2[1]) { printf("file %s not large enough (%ld)\n", file_merges[merge_id*3+1], file_size_2); exit(1); }
      data_2 = (short*)realloc(data_2, sizeof(short)*size_2[0]*size_2[1]);
      fread(data_2, size_2[0]*size_2[1], sizeof(short), fp_2);
    }
    
    if (bbox_1[2] == bbox_2[0] && size_1[0]==size_2[0]) // se1_lat == nw2_lat  (north/south connection)
    {
      printf("  north/south merge\n");
      printf("    '%35s': (%d by %d) %f %f %f %f\n", file_merges[merge_id*3], size_1[0], size_1[1], bbox_1[0], bbox_1[1], bbox_1[2], bbox_1[3]);
      printf("    '%35s': (%d by %d) %f %f %f %f\n", file_merges[merge_id*3+1], size_2[0], size_2[1], bbox_2[0], bbox_2[1], bbox_2[2], bbox_2[3]);
      size_3[0] = size_1[0];
      size_3[1] = size_1[1] + size_2[1];
      bbox_3[0] = bbox_1[0];
      bbox_3[1] = bbox_1[1];
      bbox_3[2] = bbox_2[2];
      bbox_3[3] = bbox_2[3];
      
      printf("destination file size: %d by %d\n", size_3[0], size_3[1]);
      data_3 = (short*)realloc(data_3, sizeof(short)*size_3[0]*size_3[1]);
      if (data_3 == NULL) { printf("could not malloc enough space for the new file :(\n"); exit(1); }
      
      printf("malloc'ed %ld to store (%ld + %ld)\n", sizeof(short)*size_3[0]*size_3[1], sizeof(short)*size_1[0]*size_1[1], sizeof(short)*size_2[0]*size_2[1]);
      int row_id = 0;
      int col_id = 0;
      printf("copying first file '%s', %d by %d into new file\n", file_merges[merge_id*3], size_1[0], size_1[1]);
      for (row_id = 0 ; row_id < size_1[0] ; row_id++)
        for (col_id = 0 ; col_id < size_1[1] ; col_id++)
          data_3[row_id*size_3[1]+col_id] = data_1[row_id*size_1[1]+col_id];
      
      printf("copying second file '%s', %d by %d into new file\n", file_merges[merge_id*3+1], size_1[0], size_1[1]);
      for (row_id = 0 ; row_id < size_2[0] ; row_id++)
        for (col_id = 0 ; col_id < size_2[1] ; col_id++)
          data_3[(row_id)*size_3[1]+col_id+size_1[1]] = data_2[row_id*size_2[1]+col_id];
      
    }
    else if (bbox_1[3] == bbox_2[1] && size_1[1]==size_2[1]) // se1_lng == nw2_lng  (north/south connection)
    {
      printf("  west/east merge\n");
      printf("    '%35s': (%d by %d) %f %f %f %f\n", file_merges[merge_id*3], size_1[0], size_1[1], bbox_1[0], bbox_1[1], bbox_1[2], bbox_1[3]);
      printf("    '%35s': (%d by %d) %f %f %f %f\n", file_merges[merge_id*3+1], size_2[0], size_2[1], bbox_2[0], bbox_2[1], bbox_2[2], bbox_2[3]);
      size_3[0] = size_1[0] + size_2[0];
      size_3[1] = size_1[1];
      bbox_3[0] = bbox_1[0];
      bbox_3[1] = bbox_1[1];
      bbox_3[2] = bbox_2[2];
      bbox_3[3] = bbox_2[3];
      //printf("destination file size: %d by %d\n", size_3[0], size_3[1]);
      data_3 = (short*)realloc(data_3, sizeof(short)*size_3[0]*size_3[1]);
      if (data_3 == NULL) { printf("could not malloc enough space for the new file :(\n"); exit(1); }
      
      //printf("malloc'ed %ld to store (%ld + %ld)\n", sizeof(short)*size_3[0]*size_3[1], sizeof(short)*size_1[0]*size_1[1], sizeof(short)*size_2[0]*size_2[1]);
      int row_id = 0;
      int col_id = 0;
      //printf("copying first file '%s', %d by %d into new file\n", file_merges[merge_id*3], size_1[0], size_1[1]);
      for (row_id = 0 ; row_id < size_1[0] ; row_id++)
        for (col_id = 0 ; col_id < size_1[1] ; col_id++)
          data_3[row_id*size_3[1]+col_id] = data_1[row_id*size_1[1]+col_id];
      
      //printf("copying second file '%s', %d by %d into new file\n", file_merges[merge_id*3+1], size_1[0], size_1[1]);
      for (row_id = 0 ; row_id < size_2[0] ; row_id++)
        for (col_id = 0 ; col_id < size_2[1] ; col_id++)
          data_3[(row_id+size_1[0])*size_3[1]+col_id] = data_2[row_id*size_2[1]+col_id];
      
    }
    else
    {
      printf("  unknown merge\n");
      printf("    '%35s': %f %f %f %f\n", file_merges[merge_id*3], bbox_1[0], bbox_1[1], bbox_1[2], bbox_1[3]);
      printf("    '%35s': %f %f %f %f\n", file_merges[merge_id*3+1], bbox_2[0], bbox_2[1], bbox_2[2], bbox_2[3]);
      continue;
    }
    
    printf("    '%35s': %f %f %f %f\n", file_merges[merge_id*3], bbox_1[0], bbox_1[1], bbox_1[2], bbox_1[3]);
    printf("    '%35s': %f %f %f %f\n", file_merges[merge_id*3+1], bbox_2[0], bbox_2[1], bbox_2[2], bbox_2[3]);
    printf("    '%35s': %f %f %f %f\n", file_merges[merge_id*3+2], bbox_3[0], bbox_3[1], bbox_3[2], bbox_3[3]);
    
    //int row_id = 0;
    //int col_id = 0;
    //for (row_id = 0 ; row_id < size_1[0] ; row_id++)
    //  for (col_id = 0 ; col_id < size_1[1] ; col_id++)
    //    data_3[row_id*1201+col_id] = elev_w[row_id*1201+col_id];
    //    //elev_we[(row_id+1201)*1201+col_id] = elev_e[row_id*1201+col_id];
    
    fwrite(&size_3[0], sizeof(short), 1, fp_3);
    fwrite(&size_3[1], sizeof(short), 1, fp_3);
    
    fwrite(&bbox_3[0], sizeof(float), 1, fp_3);
    fwrite(&bbox_3[1], sizeof(float), 1, fp_3);
    fwrite(&bbox_3[2], sizeof(float), 1, fp_3);
    fwrite(&bbox_3[3], sizeof(float), 1, fp_3);
    fwrite(data_3, sizeof(short), size_3[0]*size_3[1], fp_3);
    
    printf("   merged '%s, %s' into '%s' (%d by %d)\n", file_merges[merge_id*3], file_merges[merge_id*3+1], file_merges[merge_id*3+2], size_3[0], size_3[1]);
    
    fclose(fp_1);
    fclose(fp_2);
    fclose(fp_3);
  }
  free(data_1);
  free(data_2);
  free(data_3);
}