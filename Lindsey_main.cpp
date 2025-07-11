/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    <Joshua Lindsey>

- All project requirements fully met? (YES or NO):
    <Yes>

- If no, please explain what you could not get to work:
    <N/a>

- Did you do any optional enhancements? If so, please explain:
    <No>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//

/**
 * Description - Displays the program's introduction message
 */
void intro_message()
{
    cout << endl << "CSPB 1300 Image Processing Application" << endl;
}

/**
 * Description - Displays menu selection and prompts user to make selection
 * @return the user's menu selection as an int
 */
int menu_selection()
{
    int selection;
    cout << endl << "----------------------------------" << endl;
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << "0) Change image (current: sample.bmp)" << endl;
    cout << "1) Vignette" << endl;
    cout << "2) Clarendon" << endl;
    cout << "3) Grayscale" << endl;
    cout << "4) Rotate 90 degrees" << endl;
    cout << "5) Rotate multiple 90 degrees" << endl;
    cout << "6) Enlarge" << endl;
    cout << "7) High contrast" << endl;
    cout << "8) Lighten" << endl;
    cout << "9) Darken" << endl;
    cout << "10) Black, white, red, green, blue" << endl;
    cout << "----------------------------------" << endl;

    cout << endl << "Enter menu selection (Q to quit): "; // Good
    cin >> selection;
    cout << endl;
    
    return selection;
}

/**
 * Description - Sets the input filename from the user's input
 * @return the name of the initial input filename as a string
 */
string initial_input_filename()
{
    string initial_filename;
    cout << "Enter input BMP filename: "; // Good
    cin >> initial_filename;
    return initial_filename;
}

/**
 * Description - Changes the input filename from the user's input. 
 * @return the name of the new input filename as a string
 */
string process_0()
{
    string new_filename;
    cout << "Change Image selected" << endl;
    cout << "Enter new input BMP filename: " << endl;
    cin >> new_filename; 
    cout << "Successfully changed input image!" << endl;
    return new_filename;
}

/**
 * Description - Adds vignette effect to image (dark corners)
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{    
    int num_rows = image.size(); // Gets the number of rows (height) in a 2D vector named image.
    int num_cols = image[0].size(); // Gets the number of columns (i.e.) in a 2D vector named image.

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            // find the distance to the center
            double distance = sqrt( pow((col - num_rows / 2),2) + pow((row - num_cols / 2),2));
            double scaling_factor = (num_cols - distance) / num_cols;
            
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            new_new_image[row][col].red   = red_color   * scaling_factor;
            new_new_image[row][col].green = green_color * scaling_factor;
            new_new_image[row][col].blue  = blue_color  * scaling_factor;
        }
    }
    return new_new_image;
}

/**
 * Description - Process 1 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_1 function to apply Vignette, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_1_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    bool success = true;

    cout << "Vignette selected" << endl;
    cout << "Enter output BMP filename: ";
    cin >> output_filename;                

    image = read_image(input_filename); 
    new_image = process_1(image); 
    success = write_image(output_filename, new_image);
    
    if (success == true)
    { cout << "Successfully applied Vignette!" << endl; }
    else
    { cout << "Process 1 failed" << endl; } 
}

/**
 * Status == working, needs clean up
 * Description - Adds Clarendon effect to image (darks darker and lights lighter) by a scaling factor
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{    
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {          
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            double average_value = (red_color + green_color + blue_color) / 3;

            if (average_value >= 170)
            {
                new_new_image[row][col].red   = (255 - (255 - red_color)   * scaling_factor);
                new_new_image[row][col].green = (255 - (255 - green_color) * scaling_factor);
                new_new_image[row][col].blue  = (255 - (255 - blue_color)  * scaling_factor);
            }
            else if (average_value < 90)
            {
                new_new_image[row][col].red   = red_color   * scaling_factor;
                new_new_image[row][col].green = green_color * scaling_factor;
                new_new_image[row][col].blue  = blue_color  * scaling_factor;
            }
            else
            {
                new_new_image[row][col].red   = red_color;
                new_new_image[row][col].green = green_color;
                new_new_image[row][col].blue  = blue_color;
            }
        }
    }
    return new_new_image;
}

/**
 * Description - Process 2 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_2 function to apply Clarendon, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_2_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    
    cout << "Clarendon selected" << endl;
    cout << "Enter output BMP filename: ";
    cin >> output_filename;
    cout << "Enter scaling factor: "; // Use 0.3
    cin >> scaling_factor;
    
    image = read_image(input_filename); 
    new_image = process_2(image, scaling_factor);
    success = write_image(output_filename, new_image);
    
    if (success == true)
    { cout << "Successfully applied Clarendon!" << endl; }
    else
    { cout << "Process 2 failed" << endl; }
}

/**
 * Status == working, needs clean up
 * Description - Grayscale image
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{    
    int num_rows = image.size();
    int num_cols = image[0].size();

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {         
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;
            double gray_value = (red_color + green_color + blue_color) / 3;

            new_new_image[row][col].red   = gray_value;
            new_new_image[row][col].green = gray_value;
            new_new_image[row][col].blue  = gray_value;
        }
    }
    return new_new_image;
}

/**
 * Description - Process 3 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_3 function to apply Grayscale, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_3_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;

    cout << "Grayscale selected" << endl;
    cout << "Enter output BMP filename: ";
    cin >> output_filename;
    
    image = read_image(input_filename);
    new_image = process_3(image);
    success = write_image(output_filename, new_image);
    
    if (success == true)
    {cout << "Successfully applied GrayScale!" << endl;}
    else
    {cout << "Process 3 failed" << endl;}
}

/**
 * Status == working, needs clean up
 * Description - Rotates image by 90 degrees clockwise (not counter-clockwise)
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{    
    int num_rows = image.size();
    int num_cols = image[0].size();

    // Invert the rows and cols for new_new_image and the nested for loops
    vector<vector<Pixel>> new_new_image(num_cols, vector<Pixel> (num_rows));

    for (int col = 0; col < num_cols; col++)
    {
        for (int row = 0; row < num_rows; row++)
        {    
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            new_new_image[col][(num_rows - 1) - row].red   = red_color;
            new_new_image[col][(num_rows - 1) - row].green = green_color;
            new_new_image[col][(num_rows - 1) - row].blue  = blue_color;
        }
    }
    return new_new_image;
}

/**
 * Description - Process 4 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_4 function to apply Rotate 90 Degrees, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_4_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;

    cout << "Rotate 90 degrees selected" << endl;
    cout << "Enter output BMP filename: ";
    cin >> output_filename;
    
    image = read_image(input_filename);
    new_image = process_4(image); 
    success = write_image(output_filename, new_image);
    
    if (success == true)
    {cout << "Successfully applied 90 degree rotation!" << endl;}
    else
    {cout << "Process 4 failed" << endl;}   
}

/**
 * IN WORK
 * Description - Rotates image by a specified number of multiples of 90 degrees clockwise
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    int angle = number * 90;
    //cout << "the angle is: " << angle << endl;;
    
    if (angle % 360 == 0)
        { return image; }
    else if (angle % 360 == 90)
        { return process_4(image); }
    else if (angle % 360 == 180)
        { return process_4(process_4(image)); }
    else
        { return process_4(process_4(process_4(image))); }
}

/**
 * Description - Process 5 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_5 function to apply Multiple 90 degree rotation, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_5_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    int num_90_degree_rotations = 0;

    cout << "Process multiple 90 degrees selected" << endl;
    
    cout << "Enter output BMP filename: ";
    cin >> output_filename;
    cout << "Enter number of 90 degree rotations: ";
    cin >> num_90_degree_rotations;
    
    image = read_image(input_filename); 
    new_image = process_5(image, num_90_degree_rotations); 
    success = write_image(output_filename, new_image);
    
    if (success == true)
    {cout << "Successfully applied multiple 90 degree rotations!" << endl;}
    else
    {cout << "Process 5 failed" << endl;}
}

/**
 * IN WORK
 * Description - Enlarges the image in the x and y direction
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{   
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    vector<vector<Pixel>> new_new_image(num_rows * y_scale, vector<Pixel> (num_cols * x_scale));

    for (int row = 0; row < num_rows * y_scale; row++)
    {
        for (int col = 0; col < num_cols * x_scale; col++)
        {         
            int red_color = image[row/y_scale][col / x_scale].red;
            int green_color = image[row/y_scale][col / x_scale].green;
            int blue_color = image[row/y_scale][col / x_scale].blue;

            new_new_image[row][col].red   = red_color;
            new_new_image[row][col].green = green_color;
            new_new_image[row][col].blue  = blue_color;
        }
    }
    return new_new_image;
}

/**
 * Description - Process 6 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_6 function to apply Enlarge, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_6_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    int x_scale = 1; 
    int y_scale = 1; 

    cout << "Enlarge selected" << endl;
    cout << "Enter output BMP filename: ";
    cin >> output_filename;
    cout << "Enter X scale: ";
    cin >> x_scale;
    cout << "Enter Y scale: ";
    cin >> y_scale;
    
    image = read_image(input_filename); 
    new_image = process_6(image, x_scale, y_scale);
    success = write_image(output_filename, new_image);
    
    if (success == true){cout << "Successfully enlarged!" << endl;}
    else{cout << "Process 7 failed" << endl;}  
}

/**
 * IN WORK
 * Description - Convert image to high contrast (black and white only)
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
{ 
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {         
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            double gray_value = (red_color + green_color + blue_color) / 3;

            if (gray_value >= 255/2)
            {
                new_new_image[row][col].red   = 255;
                new_new_image[row][col].green = 255;
                new_new_image[row][col].blue  = 255;
            }
            else
            {
                new_new_image[row][col].red   = 0;
                new_new_image[row][col].green = 0;
                new_new_image[row][col].blue  = 0;
            }
        }
    }
    return new_new_image;
}

/**
 * Description - Process 7 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_7 function to apply High Contrast, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_7_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    int x_scale = 1; 
    int y_scale = 1; 

    cout << "High Contrast selected" << endl << "Enter output BMP filename: ";
    cin >> output_filename;
    
    image = read_image(input_filename); 
    new_image = process_7(image);
    success = write_image(output_filename, new_image);
    
    if (success == true){cout << "Successfully applied high contrast!" << endl;}
    else{cout << "Process 7 failed" << endl;}
}

/**
 * IN WORK
 * Description - Lightens image by a scaling factor
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor)
{    
    int num_rows = image.size();
    int num_cols = image[0].size(); 

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {          
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int color_sum = red_color + green_color + blue_color;

            new_new_image[row][col].red   = (255 - (255 - red_color)   * scaling_factor);
            new_new_image[row][col].green = (255 - (255 - green_color) * scaling_factor);
            new_new_image[row][col].blue  = (255 - (255 - blue_color)  * scaling_factor);        
        }
    }
    return new_new_image;
}

/**
 * Description - Process 8 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_8 function to apply Lighten, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_8_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    int x_scale = 1; 
    int y_scale = 1; 

    cout << "Lighten selected" << endl << "Enter output BMP filename: ";
    cin >> output_filename;
    cout << "Enter scaling factor: ";
    cin >> scaling_factor;
    
    image = read_image(input_filename); 
    new_image = process_8(image, scaling_factor);
    success = write_image(output_filename, new_image);
    
    if (success == true)
    {cout << "Successfully lightened!" << endl;}
    else
    {cout << "Process 8 failed" << endl;}   
}

/**
 * IN WORK
 * Description - Darkens image by a scaling factor
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)
{    
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {          
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;
            int color_sum = red_color + green_color + blue_color;

            new_new_image[row][col].red   = red_color   * scaling_factor;
            new_new_image[row][col].green = green_color * scaling_factor;
            new_new_image[row][col].blue  = blue_color  * scaling_factor;    
        }
    }
    return new_new_image;
}

/**
 * Description - Process 9 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_9 function to apply Darken, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_9_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    int x_scale = 1; 
    int y_scale = 1; 

    cout << "Darken selected" << endl << "Enter output BMP filename: ";
    cin >> output_filename;
    cout << "Enter scaling factor: ";
    cin >> scaling_factor;
    
    image = read_image(input_filename); 
    new_image = process_9(image, scaling_factor);
    success = write_image(output_filename, new_image);
    
    if (success == true)
    {cout << "Successfully darkened!" << endl;}
    else
    {cout << "Process 9 failed" << endl;} 
}

/**
 * IN WORK
 * Description - Converts image to only black, white, red, blue, and green
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{    
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    vector<vector<Pixel>> new_new_image(num_rows, vector<Pixel> (num_cols));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {          
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int color_sum = red_color + green_color + blue_color;

            if (color_sum >= 550)
            {
                new_new_image[row][col].red   = 255;
                new_new_image[row][col].green = 255;
                new_new_image[row][col].blue  = 255;
            }
            else if (color_sum <= 150)
            {
                new_new_image[row][col].red   = 0;
                new_new_image[row][col].green = 0;
                new_new_image[row][col].blue  = 0;
            }
            else if (red_color > green_color && red_color > blue_color)
            {
                new_new_image[row][col].red   = 255;
                new_new_image[row][col].green = 0;
                new_new_image[row][col].blue  = 0;
            }
            else if (green_color > red_color && green_color > blue_color)
            {
                new_new_image[row][col].red   = 0;
                new_new_image[row][col].green = 255;
                new_new_image[row][col].blue  = 0;
            }
            else
            {
                new_new_image[row][col].red   = 0;
                new_new_image[row][col].green = 0;
                new_new_image[row][col].blue  = 255;
            }           
        }
    }
    return new_new_image;
}

/**
 * Description - Process 10 Wrapper function. Takes input filename, calls read_image functions to transform the image into a vector,
 * calls process_10 function to apply Black, White, Red, Green, Blue, calls write_image fucntion to transform the new vector into a .bmp file, and prints success,
 * if the image transformation was successful.
 * @param input_filename BMP image filename
 */
void process_10_wrapper(string input_filename)
{
    string output_filename = "";
    vector<vector<Pixel>> image;
    vector<vector<Pixel>> new_image;
    double scaling_factor = 1;
    bool success = true;
    int x_scale = 1; 
    int y_scale = 1; 

    cout << "Black, White, Red, Green, Blue selected!" << endl << "Enter output BMP filename: ";
    cin >> output_filename;
    
    image = read_image(input_filename); 
    new_image = process_10(image); 
    success = write_image(output_filename, new_image);
    
    if (success == true)
    {cout << "Successfully applied Black, White, Red, Green, Blue filter!" << endl;}
    else
    {cout << "Process 10 failed" << endl;}
}

int main()
{
    // Initialize all variables required prior to calling process functions
    // All variables required for each individual process are included within each individual process function
    string input_filename = "";
    int user_selection = 0;
    bool stop = false;

    // Call functions to display intro message and prompt user for input file name 
    intro_message(); 
    input_filename = initial_input_filename();
    
    // Enumerate each process so switch statement can be used to handle user selection within while loop
    enum Processes
    {
        Change_Image = 0,           // Process 0
        Vignetee,                   // Process 1
        Clarendon,                  // Process 2
        Grayscale,                  // Process 3
        Rotate_90_Degrees,          // Process 4
        Rotate_X_90_Degrees,        // Process 5
        Enlarge,                    // Process 6
        High_Contrast,              // Process 7
        Lighten,                    // Process 8
        Darken,                     // Process 9
        Black_White_Red_Green_Blue  // Process 10
    };

    while (!stop)
    {
        user_selection = menu_selection();

        // If statement handles non-numerical inputs to quit program
        if (cin.fail())
        {
            stop = true;
            cout << "Thank you for using my program!" << endl << "Quitting..." << endl;
        }
        // else statement handles all numerical inputs for menu selection
        else
        {
            // Switch statement to handle menu options established by enumeration
            // Each case or process takes the user selection and calls a process or process_wrapper function to execute what the user wants.
            switch(user_selection)
            {
            case Change_Image: // Process 0 
                input_filename = process_0();
                break;

            case Vignetee: // Process 3
                process_1_wrapper(input_filename);
                break;

            case Clarendon: // Process 2
                process_2_wrapper(input_filename);
                break;

            case Grayscale: // Process 3
                process_3_wrapper(input_filename);
                break;

            case Rotate_90_Degrees: // Process 4 
                process_4_wrapper(input_filename);
                break;

            case Rotate_X_90_Degrees: // Process 5
                process_5_wrapper(input_filename);
                break;

            case Enlarge: // Process 6
                process_6_wrapper(input_filename);
                break;

            case High_Contrast: // Process 7
                process_7_wrapper(input_filename);
                break;

            case Lighten: // Process 8
                process_8_wrapper(input_filename);
                break;

            case Darken: // Process 9
                process_9_wrapper(input_filename);
                break;

            case Black_White_Red_Green_Blue: // Process 10
                process_10_wrapper(input_filename);
                break;

            // Default switch case handles numerical user selections that are out of bounds of the menu selection
            default:
                cout << "Invalid input. Select an option within the menu bounds" << endl; // reword
                break;
            }
        }
    }
    return 0;
}