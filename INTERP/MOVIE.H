// Header file for movie.c -- animated movie display
void RunMovie(int handle,Handle bufferHandle,int speed);
void LoadFrame(char page,int handle, Handle bufferHandle);
void DisplayFrame(int handle, Handle bufferHandle);
void DisplayBlock(char far * buffer,int xOff,int yOff,int xDim,int yDim);
void FillVideoPage(void far * xCel);
void SetVideoMode(int mode);
void ShowPage(int pageOffset);
void CopyPage(int destPage,int srcPage);
void LoadMoviePalette(int handle);
unsigned LoadCelCount(int handle);



