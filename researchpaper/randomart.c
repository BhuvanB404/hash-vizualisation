#include <stdio.h>
#include <stdint.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "stb_image.h"
#include "stb_image_write.h"
#include "nob.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"
#include <math.h>



#define WIDTH 400
#define HEIGHT 400


static Arena node_arena = {0};

typedef enum
{
    NT_X,
    NT_Y,
    NT_NUMBER,
    NT_MULTI,
    NT_TRIPLE,
    NT_ADD,
}Node_Types;

typedef struct Node_As Node_As;

typedef struct Node Node;


typedef  struct
{
    Node *lhs;
    Node *rhs;
}Node_binop;


typedef struct {
    Node *first;
    Node *second;
    Node *third;
}Node_tripple;

struct Node_As
{
float number;
Node_binop binop;
Node_tripple tri;//sbin.usr-is-merged/ binary operators
};


struct Node
{
Node_Types kind;
Node_As *as;
const char *file;
    int line;
};


Node *node_loc(const char *file, int line, Node_Types kind)
{

    Node *node = arena_alloc(&node_arena, sizeof(Node));
node->as = arena_alloc(&node_arena, sizeof(Node_As));
node->kind = kind;
node->file = file;
node->line = line;
return node;
}


Node *node_number_loc(const char *file,int line,float number)
{
    Node *node = node_loc(file, line, NT_NUMBER);
    node-> as->number = number;
    return node;

}
#define node_number(number) node_number_loc(__FILE__,__LINE__, number)

#define node_X() node_loc(__FILE__,__LINE__, NT_X)
#define node_Y() node_loc(__FILE__,__LINE__, NT_Y)

//Node *node_X_loc(const char *file,int line)

//{
   //sbin.usr-is-merged/ Node *node = arena_alloc(&node_arena, sizeof(Node));

//node->as = arena_alloc(&node_arena, sizeof(Node_As));
//node->kind =NT_X;
  //  return node;

//    return node_loc(file,line, NT_X);
//}


/*/Node *node_Y()
{
    Node *node = arena_alloc(&node_arena, sizeof(Node));
node->as = arena_alloc(&node_arena, sizeof(Node_As));
    node->kind =NT_Y;
    return node;

}
*/

Node *node_add_loc(const char *file, int line,Node *lhs,Node *rhs)
{
    Node *node  = node_loc(file,line,NT_ADD);
    node->as->binop.lhs = lhs;
    node->as->binop.rhs = rhs;
    return node;
}

#define node_add(lhs,rhs) node_add_loc(__FILE__, __LINE__, lhs,rhs)

Node *node_multi_loc(const char *file, int line, Node *lhs,Node *rhs)
{
    Node *node = node_loc(file, line, NT_MULTI);
    node->as->binop.lhs = lhs;
    node->as->binop.rhs = rhs;
    return node;
}

#define node_multi(lhs,rhs) node_multi_loc(__FILE__, __LINE__, lhs,rhs)

Node *node_tripple_loc(const char *file,int line,Node *a,Node *b, Node *c)
{
    Node *node = node_loc(file,line,NT_TRIPLE);
    node->as->tri.first  = a;
    node->as->tri.second = b;
    node->as->tri.third = c;
    return node;
}

#define node_tripple(a,b,c) node_tripple_loc(__FILE__, __LINE__, a,b,c)

void node_print(Node *node)
{
    switch(node->kind)
    {
    case NT_X:
        printf("x");
        break;
    case NT_Y:
        printf("y");
        break;
    case NT_NUMBER:
        printf("%f", node->as->number);
        break;
    case NT_MULTI:
        printf("multi(");
        node_print(node->as->binop.lhs);
        printf(",");
        node_print(node->as->binop.rhs);
        printf(")");
        break;
    case NT_TRIPLE:
        printf("(");
        node_print(node->as->tri.first);
        printf(",");
         node_print(node->as->tri.second);
        printf(",");
         node_print(node->as->tri.third);
        printf(")");
        break;
    case NT_ADD:
        printf("add(");
        node_print(node->as->binop.lhs);
        printf(",");
        node_print(node->as->binop.rhs);
        printf(")");
        break;
    default: UNREACHABLE("node print");

}
}

typedef struct{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
}RGBA32; // this is a struct which creates a 32bit struct idenitcal to 32bit rgba.

static RGBA32 pixels[WIDTH * HEIGHT]; // this is the pixel array

typedef struct {
    float x,y;
}Vec2;


typedef struct {
    float r,g, b;
}ColorV;

ColorV gray_gradiant(float x, float y)
{   
    UNUSED(y);
    return(ColorV) {
        x,
        x,
        x
    };
}


ColorV cool(float x, float y)
{

    // there is some wierdness here . Both x*y > 0 and x* y >= 0 produce the same results.
    // this neesds testing.
    if(x*y > 0){ return (ColorV){x,y,1};
    }
    else
    {
        float r = fmodf(x,y);
        return (ColorV){r,r,r};


    }
    
}
Node *eval(Node *expr, float x, float y)
{
    switch(expr->kind)
    {
        case NT_X: return node_number(x);
            break;
        case NT_Y: return node_number(y);
            break;
        case NT_NUMBER: return expr;
            break;
        case NT_ADD:
        {       Node *lhs = eval(expr->as->binop.lhs,x,y);
                if(!lhs) return NULL;

            if(lhs->kind != NT_NUMBER)
            {
                printf("%s:%d: ERROR: expected number", expr->as->binop.lhs->file,expr->as->binop.lhs->line);
                return NULL;
            }
            Node *rhs = eval(expr->as->binop.rhs, x,y);
            if(!rhs) return NULL;

            if(rhs->kind != NT_NUMBER)
            {
                printf("%s:%d: ERROR: expected number", expr->as->binop.rhs->file,expr->as->binop.rhs->line);
                return NULL;
            }
            return node_number_loc(expr->file,expr->line, lhs->as->number + rhs->as->number);
    }
        case NT_MULTI:

        {   Node *lhs = eval(expr->as->binop.lhs,x,y);
            if(!lhs) return NULL;

             if(lhs->kind != NT_NUMBER)
            {
                printf("%s:%d: ERROR: expected number", expr->as->binop.lhs->file,expr->as->binop.lhs->line);
                return NULL;
            }
            Node *rhs = eval(expr->as->binop.rhs, x,y);
            if(!rhs) return NULL;
            if(rhs->kind != NT_NUMBER)
            {
                printf("%s:%d: ERROR: expected number", expr->as->binop.rhs->file,expr->as->binop.rhs->line);
                return NULL;
            }
            return node_number_loc(expr->file,expr->line, lhs->as->number * rhs->as->number);
        }
            case NT_TRIPLE:
    {       Node *first = eval(expr->as->tri.first, x , y);
        if(!first) return NULL;
            Node *second = eval(expr->as->tri.second, x , y);
            if(!second) return NULL;
            Node *third = eval(expr->as->tri.third, x , y);
            if(!third) return NULL;

            return node_tripple_loc(expr->file,expr->line, first,second,third);
    }

    default: NOB_UNREACHABLE("eval");
    }
}

bool eval_func(Node *body, float x, float y, ColorV *c)
{
    Node *result = eval(body, x, y);
    if(result == NULL) return false;
    if(result->kind != NT_TRIPLE)
    {
        printf("%s:%d: Function must return triple\n", result->file, result->line);
        return false;
    }
    if(result->as->tri.first->kind != NT_NUMBER)
    {
        printf("%s:%d: First element must be a number\n", result->file, result->line);
        return false;
    }
    if(result->as->tri.second->kind != NT_NUMBER)
    {
        printf("%s:%d: Second element must be a number\n", result->file, result->line);
        return false;
    }
    if(result->as->tri.third->kind != NT_NUMBER)
    {
        printf("%s:%d: Third element must be a number\n", result->file, result->line);
        return false;
    }
    c->r = result->as->tri.first->as->number;
    c->g = result->as->tri.second->as->number;
    c->b = result->as->tri.third->as->number;
    
    return true;
}

bool render_thou_pixel(Node *f)  // Changed to accept Node* instead of function pointer
{
    for(size_t y = 0; y < HEIGHT; y++) {
        float ny = ((float)y/(HEIGHT-1)) * 2.0f - 1.0f;  // Normalize y to [-1, 1]
        for(size_t x = 0; x < WIDTH; x++) {
            float nx = ((float)x/(WIDTH-1)) * 2.0f - 1.0f;  // Normalize x to [-1, 1]
            ColorV c;
            
            if(!eval_func(f, nx, ny, &c)) {
                return false;
            }
              // ColorV c = eval(f,nx,ny);

            size_t index = y * WIDTH + x;
            
            // Clamp and convert to 0-255 range
            pixels[index].r = (uint8_t)((c.r + 1.0f) * 0.5f * 255.0f);
            pixels[index].g = (uint8_t)((c.g + 1.0f) * 0.5f * 255.0f);
            pixels[index].b = (uint8_t)((c.b + 1.0f) * 0.5f * 255.0f);
            pixels[index].a = 255;
        }
    }
    return true;
}


int main()
{
    

 //render_thou_pixel(gray_);
//  render_thou_pixel(cool);
   bool ok =  render_thou_pixel(
        node_tripple(node_X(),node_Y(),node_number(0.5)));

    if(!ok) return 1;
    const char *output = "output.png";
    if(!stbi_write_png(output, WIDTH,HEIGHT , 4 , pixels, WIDTH*sizeof(RGBA32)) )
 {
    nob_log(NOB_ERROR, "Could  not save image %s",output);
    return 1;
}

 nob_log(INFO, "Generated %s", output);
    return 0;
}
