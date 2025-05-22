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
#include <time.h> // For time(0)

#define WIDTH 800
#define HEIGHT 800

static Arena node_arena = {0};
static Arena static_arena = {0};

typedef enum
{
    NT_X,
    NT_Y,
    NT_RANDOM,
    NT_RULE,
    NT_RULE_OR,

    NT_NUMBER,
    NT_MULTI,
    NT_ADD,
    NT_MOD,

    NT_BOOLEAN,
    NT_GREATER,

    NT_TRIPLE, 

    NT_IF,
    NT_COUNT,
}Node_Types;

typedef struct Node Node;

typedef struct {
    Node *lhs;
    Node *rhs;
}Node_binop;

typedef struct {
    Node *first;
    Node *second;
    Node *third;
}Node_tripple;

typedef struct {
    Node *condition;
    Node *then;
    Node *elsse;
}Node_If;

typedef union {
    float number;
    bool boolean;
    Node_binop binop;
    Node_tripple tri;
    Node_If iff;
    int rule;
} Node_As;

struct Node
{
    Node_Types kind;
    Node_As as;
    const char *file;
    int line;
};

Node *node_loc(const char *file, int line, Arena *arena, Node_Types kind)
{
    Node *node = arena_alloc(arena, sizeof(Node));
    node->kind = kind;
    node->file = file;
    node->line = line;
    return node;
}

Node *node_unary_loc(const char *file, int line, Arena *arena, Node_Types kind, Node *unary)
{
    Node *node = node_loc(file, line, arena, kind);
    return node;
}

Node *node_binop_loc(const char *file, int line, Arena *arena, Node_Types kind, Node *lhs, Node *rhs)
{
    Node *node = node_loc(file, line, arena, kind);
    node->as.binop.lhs = lhs;
    node->as.binop.rhs = rhs;
    return node;
}

Node *node_number_loc(const char *file, int line, Arena *arena, float number)
{
    Node *node = node_loc(file, line, arena, NT_NUMBER);
    node->as.number = number;
    return node;
}
#define node_number(arena, number) node_number_loc(__FILE__, __LINE__, arena, number)

Node *node_rule_loc(const char *file, int line, Arena *arena, int rule)
{
    Node *node = node_loc(file, line, arena, NT_RULE);
    node->as.rule = rule;
    return node;
}
#define node_rule(arena, rule) node_rule_loc(__FILE__, __LINE__, arena, rule)

Node *node_boolean_loc(const char *file, int line, Arena *arena, bool boolean)
{
    Node *node = node_loc(file, line, arena, NT_BOOLEAN);
    node->as.boolean = boolean;
    return node;
}
#define node_boolean(arena, boolean) node_boolean_loc(__FILE__, __LINE__, arena, boolean)

#define node_x(arena)      node_loc(__FILE__, __LINE__, arena, NT_X)
#define node_y(arena)      node_loc(__FILE__, __LINE__, arena, NT_Y)
#define node_random(arena) node_loc(__FILE__, __LINE__, arena, NT_RANDOM)

#define node_add(arena, lhs, rhs)  node_binop_loc(__FILE__, __LINE__, arena, NT_ADD, lhs, rhs)
#define node_multi(arena, lhs, rhs) node_binop_loc(__FILE__, __LINE__, arena, NT_MULTI, lhs, rhs)
#define node_mod(arena, lhs, rhs)  node_binop_loc(__FILE__, __LINE__, arena, NT_MOD, lhs, rhs)
#define node_gt(arena, lhs, rhs)   node_binop_loc(__FILE__, __LINE__, arena, NT_GREATER, lhs, rhs)

Node *node_tripple_loc(const char *file, int line, Arena *arena, Node *first, Node *second, Node *third)
{
    Node *node = node_loc(file, line, arena, NT_TRIPLE);
    node->as.tri.first  = first;
    node->as.tri.second = second;
    node->as.tri.third  = third;
    return node;
}
#define node_tripple(arena, first, second, third) node_tripple_loc(__FILE__, __LINE__, arena, first, second, third)

Node *node_if_loc(const char *file, int line, Arena *arena, Node *cond, Node *then, Node *elze)
{
    Node *node = node_loc(file, line, arena, NT_IF);
    node->as.iff.condition = cond;
    node->as.iff.then = then;
    node->as.iff.elsse = elze;
    return node;
}
#define node_if(arena, cond, then, elze) node_if_loc(__FILE__, __LINE__, arena, cond, then, elze)

void node_print(Node *node)
{
    switch(node->kind)
    {
    case NT_RULE_OR:
        // Handle or print something, or just break;
        break;
    
    case NT_X:
        printf("x");
        break;
    case NT_Y:
        printf("y");
        break;
    case NT_NUMBER:
        printf("%f", node->as.number);
        break;
    case NT_MULTI:
        printf("multi(");
        node_print(node->as.binop.lhs);
        printf(", ");
        node_print(node->as.binop.rhs);
        printf(")");
        break;
    case NT_MOD:
        printf("mod(");
        node_print(node->as.binop.lhs);
        printf(", ");
        node_print(node->as.binop.rhs);
        printf(")");
        break;
    case NT_TRIPLE:
        printf("(");
        node_print(node->as.tri.first);
        printf(", ");
        node_print(node->as.tri.second);
        printf(", ");
        node_print(node->as.tri.third);
        printf(")");
        break;
    case NT_ADD:
        printf("add(");
        node_print(node->as.binop.lhs);
        printf(", ");
        node_print(node->as.binop.rhs);
        printf(")");
        break;
    case NT_BOOLEAN:
        printf("%s", node->as.boolean ? "true" : "false");
        break; 
    case NT_GREATER:
        printf("greater(");
        node_print(node->as.binop.lhs);
        printf(", ");
        node_print(node->as.binop.rhs);
        printf(")");
        break;  
    case NT_IF:
        printf("if ");
        node_print(node->as.iff.condition);
        printf(" then ");
        node_print(node->as.iff.then);
        printf(" else ");
        node_print(node->as.iff.elsse);
        break;  
    case NT_RULE:
        printf("rule(%d)", node->as.rule);
        break;
    case NT_RANDOM:
        printf("random");
        break;
    case NT_COUNT:
    default: NOB_UNREACHABLE("node print");
    }
}

typedef struct{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
}RGBA32; creates some 32 bit standard image 
static RGBA32 pixels[WIDTH * HEIGHT]; // this is scrreen

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
    if(x*y > 0){ 
        return (ColorV){x,y,1};
    }
    else
    {
        float r = fmodf(x,y);
        return (ColorV){r,r,r};
    }
}

bool expect_number(Node *expr)
{
    if (expr->kind != NT_NUMBER) {
        printf("%s:%d: ERROR: expected number\n", expr->file, expr->line);
        return false;
    }
    return true;
}

bool expect_boolean(Node *expr)
{
    if (expr->kind != NT_BOOLEAN) {
        printf("%s:%d: ERROR: expected boolean\n", expr->file, expr->line);
        return false;
    }
    return true;
}

bool expect_triple(Node *expr)
{
    if (expr->kind != NT_TRIPLE) {
        printf("%s:%d: ERROR: expected triple\n", expr->file, expr->line);
        return false;
    }
    return true;
}

Node *eval(Node *expr, Arena *arena, float x, float y)
{
    if (!expr) {
        return NULL;
    }
    
    switch(expr->kind)
    {
    case NT_RULE_OR:
        // Handle or print something, or just break;
        break;
    
    case NT_X: 
        return node_number_loc(expr->file, expr->line, arena, x);
    case NT_Y: 
        return node_number_loc(expr->file, expr->line, arena, y);
    case NT_BOOLEAN:
    case NT_NUMBER: 
        return expr;
    case NT_RULE:
        printf("%s:%d: ERROR: cannot evalute grammar rule %d\n", expr->file, expr->line, expr->as.rule);
        return NULL;
    case NT_RANDOM:
        printf("%s:%d: ERROR: cannot evaluate a node that is valid only for grammar definitions\n", expr->file, expr->line);
        return NULL;
    case NT_ADD: {
        Node *lhs = eval(expr->as.binop.lhs, arena, x, y);
        if (!lhs) return NULL;
        if (!expect_number(lhs)) return NULL;
        
        Node *rhs = eval(expr->as.binop.rhs, arena, x, y);
        if (!rhs) return NULL;
        if (!expect_number(rhs)) return NULL;
        
        return node_number_loc(expr->file, expr->line, arena, lhs->as.number + rhs->as.number);
    }
    case NT_MULTI: {
        Node *lhs = eval(expr->as.binop.lhs, arena, x, y);
        if (!lhs) return NULL;
        if (!expect_number(lhs)) return NULL;
        
        Node *rhs = eval(expr->as.binop.rhs, arena, x, y);
        if (!rhs) return NULL;
        if (!expect_number(rhs)) return NULL;
        
        return node_number_loc(expr->file, expr->line, arena, lhs->as.number * rhs->as.number);
    }
    case NT_MOD: {
        Node *lhs = eval(expr->as.binop.lhs, arena, x, y);
        if (!lhs) return NULL;
        if (!expect_number(lhs)) return NULL;
        
        Node *rhs = eval(expr->as.binop.rhs, arena, x, y);
        if (!rhs) return NULL;
        if (!expect_number(rhs)) return NULL;
        
        return node_number_loc(expr->file, expr->line, arena, fmodf(lhs->as.number, rhs->as.number));
    }
    case NT_TRIPLE: {
        Node *first = eval(expr->as.tri.first, arena, x, y);
        if (!first) return NULL;
        
        Node *second = eval(expr->as.tri.second, arena, x, y);
        if (!second) return NULL;
        
        Node *third = eval(expr->as.tri.third, arena, x, y);
        if (!third) return NULL;
        
        return node_tripple_loc(expr->file, expr->line, arena, first, second, third);
    }
    case NT_GREATER: {
        Node *lhs = eval(expr->as.binop.lhs, arena, x, y);
        if (!lhs) return NULL;
        if (!expect_number(lhs)) return NULL;
        
        Node *rhs = eval(expr->as.binop.rhs, arena, x, y);
        if (!rhs) return NULL;
        if (!expect_number(rhs)) return NULL;
        
        return node_boolean_loc(expr->file, expr->line, arena, lhs->as.number > rhs->as.number);
    }
    case NT_IF: {
        Node *condition = eval(expr->as.iff.condition, arena, x, y);
        if (!condition) return NULL;
        if (!expect_boolean(condition)) return NULL;
        
        if (condition->as.boolean) {
            return eval(expr->as.iff.then, arena, x, y);
        } else {
            return eval(expr->as.iff.elsse, arena, x, y);
        }
    }
    case NT_COUNT:
    default: NOB_UNREACHABLE("eval");
    }
    return NULL;
}

bool eval_func(Node *body, Arena *arena, float x, float y, ColorV *c)
{
    Node *result = eval(body, arena, x, y);
    if (!result) return false;
    if (!expect_triple(result)) return false;
    if (!expect_number(result->as.tri.first)) return false;
    if (!expect_number(result->as.tri.second)) return false;
    if (!expect_number(result->as.tri.third)) return false;
    
    c->r = result->as.tri.first->as.number;
    c->g = result->as.tri.second->as.number;
    c->b = result->as.tri.third->as.number;
    
    return true;
}

bool render_thou_pixel(Node *f)
{
    Arena arena = {0};
    for (size_t y = 0; y < HEIGHT; y++) {
        float ny = ((float)y/(HEIGHT-1)) * 2.0f - 1.0f;  // Normalize y to [-1, 1] or functions break and working with non standard values if fucking frustrating
        for (size_t x = 0; x < WIDTH; x++) {
            float nx = ((float)x/(WIDTH-1)) * 2.0f - 1.0f;  // same same as above . NOTE TO SELF ALWAYS TRY TO NORMALIZE AND WORK
            ColorV c;
            
            if (!eval_func(f, &arena, nx, ny, &c)) {
                arena_reset(&arena);
                return false;
            }
            arena_reset(&arena);

            size_t index = y * WIDTH + x;
            
            // convert to  0-255 range to generate imagea
            pixels[index].r = (uint8_t)((c.r + 1.0f) * 0.5f * 255.0f);
            pixels[index].g = (uint8_t)((c.g + 1.0f) * 0.5f * 255.0f);
            pixels[index].b = (uint8_t)((c.b + 1.0f) * 0.5f * 255.0f);
            pixels[index].a = 255;
        }
    }
    return true;
}

typedef struct {
    Node *node;
    float probability;
} Grammar_Branch;

typedef struct {
    Grammar_Branch *items;
    size_t capacity;
    size_t count;
} Grammar_Branches;

typedef struct {
    Grammar_Branches *items;
    size_t capacity;
    size_t count;
} Grammar;

float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

Node *gen_rule(Grammar grammar, Arena *arena, size_t rule, int depth);

Node *gen_node(Grammar grammar, Arena *arena, Node *node, int depth)
{
    switch (node->kind) {
    case NT_X:
    case NT_Y:
    case NT_NUMBER:
    case NT_BOOLEAN:
        return node;

    case NT_ADD:
    case NT_MULTI:
    case NT_MOD:
    case NT_GREATER: {
        Node *lhs = gen_node(grammar, arena, node->as.binop.lhs, depth -1  );
        if (!lhs) return NULL;
        Node *rhs = gen_node(grammar, arena, node->as.binop.rhs, depth  - 1);
        if (!rhs) return NULL;
        return node_binop_loc(node->file, node->line, arena, node->kind, lhs, rhs);
    }

    case NT_TRIPLE: {
        Node *first  = gen_node(grammar, arena, node->as.tri.first, depth -1 );
        if (!first) return NULL;
        Node *second = gen_node(grammar, arena, node->as.tri.second, depth- 1 );
        if (!second) return NULL;
        Node *third  = gen_node(grammar, arena, node->as.tri.third, depth  - 1);
        if (!third) return NULL;
        return node_tripple_loc(node->file, node->line, arena, first, second, third );
    }
    
    case NT_IF: {
        Node *cond = gen_node(grammar, arena, node->as.iff.condition, depth - 1 );
        if (!cond) return NULL;
        Node *then = gen_node(grammar, arena, node->as.iff.then, depth - 1);
        if (!then) return NULL;
        Node *elsse = gen_node(grammar, arena, node->as.iff.elsse, depth - 1);
        if (!elsse) return NULL;
        return node_if_loc(node->file, node->line, arena, cond, then, elsse);
    }

    case NT_RULE:
        return gen_rule(grammar, arena, node->as.rule, depth - 1 );

    case NT_RANDOM:
        return node_number_loc(node->file, node->line, arena, rand_float()*2.0f - 1.0f);

    case NT_COUNT:
    case NT_RULE_OR:
    default:
        NOB_UNREACHABLE("gen_node");
    }
}

#define GEN_RULE_MAX_ATTEMPTS 100

Node *gen_rule(Grammar grammar, Arena *arena, size_t rule, int depth)
{
    if (depth <= 0) return NULL;

    assert(rule < grammar.count);

    Grammar_Branches *branches = &grammar.items[rule];
    assert(branches->count > 0);

    Node *node = NULL;
    for (size_t attempts = 0; node == NULL && attempts < GEN_RULE_MAX_ATTEMPTS; ++attempts) {
        // [0......][...][...1] // this is array created 
        float p = rand_float();
        float t = 0.0f;
        for (size_t i = 0; i < branches->count; ++i) {
            t += branches->items[i].probability;
            if (t >= p) {
                node = gen_node(grammar, arena, branches->items[i].node, depth - 1);
                break;
            }
        }
    }
    return node;
}

#define node_print_ln(node) (node_print(node), printf("\n"))

size_t arch[] = {2, 28, 28, 9, 3};

int main()
{
    srand(time(0));
    Grammar grammar = {0};
    Grammar_Branches branches = {0};
    int e = 0;
    int a = 1;
    int c = 2;

    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_tripple(&static_arena, node_rule(&static_arena, c), node_rule(&static_arena, c), node_rule(&static_arena, c)),
        .probability = 1.0f
    }));
    arena_da_append(&static_arena, &grammar, branches);
    memset(&branches, 0, sizeof(branches));

    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_random(&static_arena),
        .probability = 1.0/3.0,
    }));
    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_x(&static_arena),
        .probability = 1.0/3.0,
    }));
    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_y(&static_arena),
        .probability = 1.0/3.0,
    }));
    arena_da_append(&static_arena, &grammar, branches);
    memset(&branches, 0, sizeof(branches));

    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_rule(&static_arena, a),
        .probability = 1.f/4.f,
    }));
    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_add(&static_arena, node_rule(&static_arena, c), node_rule(&static_arena, c)),
        .probability = 3.f/8.f,
    }));
    arena_da_append(&static_arena, &branches, ((Grammar_Branch) {
        .node = node_multi(&static_arena, node_rule(&static_arena, c), node_rule(&static_arena, c)),
        .probability = 3.f/8.f,
    }));
    arena_da_append(&static_arena, &grammar, branches);
    memset(&branches, 0, sizeof(branches));

    Node *f = gen_rule(grammar, &static_arena, e, 30);
    if (!f) {
        fprintf(stderr, "ERROR: the shitty generation process failedfd  could not terminate\n");
        return 1;
    }
    node_print_ln(f);

    bool ok = render_thou_pixel(f);

    if (!ok) return 1;
    const char *output_path = "output.png";
    if (!stbi_write_png(output_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(RGBA32))) {
        nob_log(ERROR, "Could not save image %s", output_path);
        return 1;
    }
    nob_log(INFO, "Generated %s", output_path);
    return 0;
}