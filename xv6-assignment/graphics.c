#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "stdbool.h"
#include "rect.h"
#include "devicecontext.h"

// Constants for video memory and screen properties
#define starting_memory_address 0xA0000
#define screen_width 320
#define screen_height 200
#define white_colour 15
#define black_colour 0
#define maximum_device_contexts 20

// Array of device contexts to manage different graphical elements
struct device_context device_context_collection[maximum_device_contexts];
// Array to track the usage status of each device context
bool is_device_context_in_use[maximum_device_contexts] = {false};


// Helper function to convert a physical address to a virtual address
static uchar* convert_physical_to_virtual_memory(uint physical_address) {
    return (uchar*)(physical_address + KERNBASE);
}

//Function to clear the entire 320x200x256 screen by setting all pixels to black
void clear320x200x256() {
	// Obtain the virtual address of the video memory
    unsigned char *video_memory_address = convert_physical_to_virtual_memory(starting_memory_address);
	// Calculate the total number of pixels on the screen
    int total_number_of_pixels = screen_width * screen_height;
	// Loop through each pixel and set its color to black
	for (int i = 0; i < total_number_of_pixels; i++) {
        video_memory_address[i] = black_colour;
    }
}

int sys_setpixel(void) {
	int hdc, x, y;
	// Retrieve arguments from the system call
    if (argint(0, &hdc) < 0) {
        return -1;
    }
    if (argint(1, &x) < 0) {
        return -1;
    }
    if (argint(2, &y) < 0) {
        return -1;
    }


    // Ensure that the specified coordinates are within the screen boundaries
	if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
		// If the coordinates are out of bounds, return an error code (-1)
		return -1; 
	}

	// Translate the physical address of the video memory to its virtual address
	unsigned char *video_memory_address = convert_physical_to_virtual_memory(starting_memory_address);
	// Calculate the linear offset in the video memory corresponding to the pixel at (x, y)
	unsigned int linear_offset = y * screen_width + x;
	// Set the color of the pixel at the calculated offset to white
	video_memory_address[linear_offset] = white_colour;

	// Return 0 to indicate a successful operation
	return 0; 
}

int sys_moveto(void) {
	int hdc, x, y;
    // Retrieve arguments from the system call
    if (argint(0, &hdc) < 0) {
        return -1;
    }
    if (argint(1, &x) < 0) {
        return -1;
    }
    if (argint(2, &y) < 0) {
        return -1;
    }

    // Ensure that the x-coordinate is within the screen boundaries
    if (x < 0) {
        x = 0;  // Set x to 0 if it's less than 0
    }
    if (x >= screen_width) {
        x = screen_width - 1;  // Set x to the maximum width - 1 if it's greater than or equal to the screen width
    }
    // Ensure that the y-coordinate is within the screen boundaries
    if (y < 0) {
        y = 0;  // Set y to 0 if it's less than 
    }
    if (y >= screen_height) {
        y = screen_height - 1;  // Set y to the maximum height - 1 if it's greater than or equal to the screen height
    }

    // Update the current graphics position
    device_context_collection[hdc].current_x_coordinates = x;
    device_context_collection[hdc].current_y_coordinates = y;

    // Return 0 to indicate a successful operation
    return 0;
}

static int abs(int value) {
	// Check if the given value is negative
    if (value < 0) {
        // If negative, return the negation of the value to make it positive
        return -value;
    } 
    else {
        // If non-negative, return the value itself (no change needed)
        return value;
    }
}

int sys_lineto(void) {
	int hdc, x1, y1;
    // Retrieve arguments from the system call
    if (argint(0, &hdc) < 0) {
        return -1;
    }
    if (argint(1, &x1) < 0) {
        return -1;
    }
    if (argint(2, &y1) < 0) {
        return -1;
    }

    //Clip coordinates to screen boundaries
    if (x1 < 0) {
        x1 = 0;
    }
    if (x1 >= screen_width) {
        x1 = screen_width - 1;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (y1 >= screen_height) {
        y1 = screen_height - 1;
    }
	
	// Initialize variables based on current coordinates and target coordinates
	int x0 = device_context_collection[hdc].current_x_coordinates;
	int y0 = device_context_collection[hdc].current_y_coordinates;
	int dx = abs(x1 - x0);	
	int sx = x0 < x1? 1 : -1; // Determine the sign of the x-direction
	int dy = -abs(y1 - y0);	
	int sy = y0 < y1 ? 1 : -1; // Determine the sign of the y-direction
	int error = dx + dy;
	int e2;
	
	// Iterate over the line using Bresenham's line drawing algorithm
	while (true) {
		// Check if the current coordinates (x0, y0) are within the screen boundaries
        if (x0 >= 0 && x0 < screen_width && y0 >= 0 && y0 < screen_height) {
            unsigned char *video_memory_address = convert_physical_to_virtual_memory(starting_memory_address);
            video_memory_address[y0 * screen_width + x0] = device_context_collection[hdc].current_pen_colour;
        }

		// Check if the current coordinates match the target coordinates
        if (x0 == x1 && y0 == y1) {
			break;
		}
        e2 = 2 * error;
		
		// Update the error term and x-coordinate based on the algorithm
        if (e2 >= dy) { 
			error = error + dy; 
			x0 = x0 + sx; 
		}

		// Update the error term and y-coordinate based on the algorithm
        if (e2 <= dx) { 
			error = error + dx; 
			y0 = y0 + sy; 
		}
	}

	// Update the current coordinates to the target coordinates
	device_context_collection[hdc].current_x_coordinates = x1;
	device_context_collection[hdc].current_y_coordinates = y1;

	// Return 0 to indicate a successful operation
	return 0;
}

static void outb(ushort port, uchar value)
{
    // The %0 and %1 are placeholders for operands, which will be replaced by the specified values.
    // The "a"(value) specifies that the 'value' parameter should be placed in the 'eax' register.
    // The "d"(port) specifies that the 'port' parameter should be placed in the 'edx' register.
    asm volatile("outb %0, %1" : : "a"(value), "d"(port));
}

int sys_setpencolour(void) {
    int index, r, g, b;
    // Extracting integer arguments from user space using the argint function.
    if (argint(0, &index) < 0) {
        return -1;
    }
    if (argint(1, &r) < 0) {
        return -1;
    }
    if (argint(2, &g) < 0) {
        return -1;
    }
    if (argint(3, &b) < 0) {
        return -1;
    }

    // Check if the colour index is within the valid range (16 to 255).
    // If not, return -1 to indicate an error.
    if (index < 16 || index > 255) {
        return -1;
    }

    // Ensure that each colour component (r, g, b) is within the valid range (0 to 63).
    // If a component is below the valid range, set it to the minimum value (0).
    // If a component is above the valid range, set it to the maximum value (63).
    if (r < 0) {
        r = 0;
    }
    if (r > 63) {
        r = 63;
    }
    
    if (g < 0) {
        g = 0;
    }
    if (g > 63) {
        g = 63;
    }

    if (b < 0) {
        b = 0;
    }
    if (b > 63) {
        b = 63;
    }

    // Set the colour palette index using the outb function.
    outb(0x3C8, index);
    // Set the red, green, and blue components using the outb function.
    outb(0x3C9, r);
    outb(0x3C9, g);
    outb(0x3C9, b);

    // Return 0 to indicate successful execution.
    return 0;
}

int sys_selectpen(void) {
    int hdc, index;
    // Extracting integer arguments from user space using the argint function.
    if (argint(0, &hdc) < 0) {
        return -1;
    }
    if (argint(1, &index) < 0) {
        return -1;
    }

    // Check if the specified pen index is within the valid range [16, 255]
    if (index < 16 || index > 255) {
        // Return an error code for an invalid pen index
        return -1;  
    }

    // Save the current pen colour index and update it with the specified index
    device_context_collection[hdc].current_pen_colour = index;

    // Return 0 to indicate succesfful execution
    return 0;
}

int sys_fillrect(void) {
    int hdc;               
    struct rect *r;
    // Extract the device context and rectangle coordinates from system arguments
    if (argint(0, &hdc) < 0) {
        return -1;
    }
    if (argptr(1, (void *)&r, sizeof(r)) < 0) {
        // Return -1 for an error in extracting the rectangle pointer
        return -1;  
    }

    // Clip the coordinates to the screen boundaries
    if (r->left < 0) {
        r->left = 0;
    }
    if (r->right >= screen_width) {
        r->right = screen_width - 1;
    }
    if (r->top < 0) {
        r->top = 0;
    }
    if (r->bottom >= screen_height) {
        r->bottom = screen_height - 1;
    }

    // Check for an invalid rectangle
    if (r->left > r->right || r->top > r->bottom) {
        // Return -1 for an invalid rectangle
        return -1;  
    }

    // Convert the starting memory address to a virtual memory address
    unsigned char *video_memory_address = convert_physical_to_virtual_memory(starting_memory_address);

    // Loop through each pixel within the clipped rectangle and set its colour to the current pen colour
    for (int j = r->top; j <= r->bottom; j++) {
        for (int k = r->left; k <= r->right; k++) {
            video_memory_address[j * screen_width + k] = device_context_collection[hdc].current_pen_colour;
        }
    }

    // Return 0 to indicate success
    return 0;
}

int sys_beginpaint(void) {
    int hwnd;
    // Extract the device context and rectangle coordinates from system arguments
    if (argint(0, &hwnd) < 0) {
        return -1;
    }

    for (int i = 0; i < maximum_device_contexts; i++) {
        // Check if the current device context is already in use
        if (is_device_context_in_use[i]) {
            // Skip to the next iteration if the device context is in use
            continue;
        }
        // Mark the current device context as in use
        is_device_context_in_use[i] = true;
        // Initialize the current device context's coordinates and pen color
        device_context_collection[i].current_x_coordinates = 0;
        device_context_collection[i].current_y_coordinates = 0;
        device_context_collection[i].current_pen_colour = white_colour;
        // Return the index of the allocated device context
        return i;
    }
    // Return -1 if no available device context is found
    return -1;
}

int sys_endpaint(void) {
    int hdc;
    // Extract the device context and rectangle coordinates from system arguments
    if (argint(0, &hdc) < 0) {
        return -1;
    }

    // Check if the retrieved device context index is valid and in use
    if (hdc >= 0 && hdc < maximum_device_contexts && is_device_context_in_use[hdc]) {
        // Initialize the properties of the specified device context
        device_context_collection[hdc].current_x_coordinates = 0;
        device_context_collection[hdc].current_y_coordinates = 0;
        device_context_collection[hdc].current_pen_colour = white_colour;
        // Mark the device context as not in use
        is_device_context_in_use[hdc] = false;
        // Return 0 to indicate success
        return 0;
    }
    // Return -1 if the device context index is invalid or not in use
    return -1;
}
