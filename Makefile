
all: dem_reader

merge:
	g++ merge_dems.c -o merge_dems

dem_reader:
	g++ main.c -o dem_reader