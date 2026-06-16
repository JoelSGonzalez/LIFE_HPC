#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <omp.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define RULE_SIZE 3
#define VIRTUAL_MAX_COLS 71
#define MAX_COLS VIRTUAL_MAX_COLS + 2
#define FILE_MAX_LINES 27

#define MIN_GRID_SIZE 3
#define MIN_GEN 1
#define SEED 25032002

typedef struct
{
    size_t size;
    size_t generations;
	int print;
} Config;

// NEW Cell Lives (FAST)
int cell_lives_fast(int cell, const int live_cells, const int rules[RULE_SIZE])
{
    return (cell == 1 && (live_cells >= rules[1] && live_cells <= rules[0])) ||
		   (cell == 0 && live_cells == rules[2]); // 3 leituras
}

// NEW Copy World
void copy_world(int **world1, size_t rows_count, size_t cols_count, int **world2)
{
    for (size_t row = 1; row <= rows_count; row++)
    {
        for (size_t col = 1; col <= cols_count; col++)
        {
            world1[row][col] = world2[row][col]; // N² escritas + N² Leituras
        }
    }
}

// NEW Update World
void update_world(int **world, size_t rows_count, size_t cols_count,
                  int **world_aux, const int rules[RULE_SIZE])
{
    for (size_t row = 1; row <= rows_count; row++)
    {
        for (size_t col = 1; col <= cols_count; col++)
        {
            int live_neighbours = 0;
            int cell = world[row][col];

            for (int i = -1; i <= 1; i++)
            {
                size_t r = (row + i - 1 + rows_count) % rows_count + 1; // 3*(1 Escritas 5 Flops)*(N²)

                for (int j = -1; j <= 1; j++)
                {
                    size_t c = (col + j - 1 + cols_count) % cols_count + 1; // 9*(1 Escrita  5 Flops)*(N²)

                    live_neighbours += world[r][c]; // 9*( 1 Leitura 1 Escritas 1 Flop)*(N²) 
                }
            }

            live_neighbours -= cell; // (1 Escrita)*(N²)

            world_aux[row][col] = cell_lives_fast(cell, live_neighbours, rules); // (1 Escrita + clf)*(N²)
        }
    }

    copy_world(world, rows_count, cols_count, world_aux);
}

// NEW Update World N Generations
void update_world_n_generations(size_t n, int **world, size_t rows_count, size_t cols_count, int **world_aux, const int rule[RULE_SIZE])
{
    if (n <= 0)
        return;

    for (int i = 0; i < n; i++)
        update_world(world, rows_count, cols_count, world_aux, rule);
}

void fprint_world(int **world, size_t rows_count, size_t cols_count, FILE *__restrict__ __stream)
{
    for (size_t i = 1; i <= rows_count; i++)
    {
        for (size_t j = 1; j <= cols_count; j++)
        {
            fputs(world[i][j] ? "X " : ". ", __stream);
        }
        fputc('\n', __stream);
    }
}

void print_world(int **world, size_t rows_count, size_t cols_count)
{
    fprint_world(world, rows_count, cols_count, stdout);
}

unsigned int parse_uint(const char *str)
{
    char *end;
    errno = 0;

    unsigned long val = strtoul(str, &end, 10);

    if (errno != 0 || *end != '\0')
    {
        fprintf(stderr, "Error: invalid number '%s'\n", str);
        exit(1);
    }

    if (val > UINT_MAX)
    {
        fprintf(stderr, "Error: value too large\n");
        exit(1);
    }

    return (unsigned int)val;
}

Config get_config(int argc, char **argv)
{
    Config cfg = {0, 0, 0};
    int opt;

    opterr = 0;

    while ((opt = getopt(argc, argv, ":s:g:p")) != -1)
    {
        switch (opt)
        {
            case 's':
                cfg.size = parse_uint(optarg);
                break;

            case 'g':
                cfg.generations = parse_uint(optarg);
                break;

            case 'p':
                cfg.print = 1;
                break;

            case ':':
                fprintf(stderr, "Error: missing value for -%c\n", optopt);
                exit(1);

            case '?':
                fprintf(stderr, "Error: invalid option -%c\n", optopt);
                exit(1);
        }
    }

    if (cfg.size < MIN_GRID_SIZE || cfg.generations < MIN_GEN)
    {
        fprintf(stderr,
            "Error: invalid parameters. Use -s >= 3 and -g >= 1\n");
        exit(1);
    }

    return cfg;
}

int **init_world(size_t size, int fill)
{
    size_t n = size + 2; //halo, padding

    int **world = malloc(n * sizeof(int *));
    if (!world) 
    {
        fprintf(stderr, "Failed to alloc WORLD!");
        exit(1);
    }
    int *data = calloc(n * n, sizeof(int));
    if (!data) 
    {
        fprintf(stderr, "Failed to alloc DATA");
        exit(1);
    }

    for (size_t i = 0; i < n; i++)
    {
        world[i] = &data[i * n];
    }

    if (!fill) return world;

    srand(SEED);

    for (size_t i = 1; i <= size; i++)
    {
        for (size_t j = 1; j <= size; j++)
        {
            world[i][j] = rand() & 1;
        }
    }

    return world;
}

void free_world(int **world)
{
    if (!world)
        return;

    free(world[0]); 
    free(world);
}

int main(int argc, char **argv)
{
    Config cfg = get_config(argc, argv);
    const int base_rules[RULE_SIZE] = {3,2,3};

    int **world = init_world(cfg.size, 1);
    int **aux = init_world(cfg.size, 0);

    double start_time, finish_time, exec_time;
    
    if (world == NULL||aux==NULL) 
    {
        fprintf(stderr, "Failed to initialize\n");
        exit(1);
    }
    
    
    if(!cfg.print)
    {
        start_time = omp_get_wtime();
        update_world_n_generations(cfg.generations, world, cfg.size, cfg.size, aux, base_rules);
        finish_time = omp_get_wtime();
        exec_time = finish_time - start_time;
    }
    else 
    {
        print_world(world, cfg.size, cfg.size);
        puts("\n");
        for (int i = 0; i < cfg.generations; i++)
        {
            update_world(world, cfg.size, cfg.size, aux, base_rules);
            print_world(world, cfg.size, cfg.size);
            puts("\n");
        }
    }
    
    
    
    free_world(world);

    puts("\n***SUCESS***\n");
    printf("World Size = %dx%d\n", cfg.size, cfg.size);
    printf("Generations = %d\n", cfg.generations);
    printf("Runtime = %.6lf seconds\n", exec_time);
    return 0;
}