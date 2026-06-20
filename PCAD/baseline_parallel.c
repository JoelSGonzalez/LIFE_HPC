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

// NEW Cell Lives
int cell_lives(const int submatrix[3][3], const int rules[RULE_SIZE])
{
	int cell = submatrix[1][1];
	int live_cells = 0;

	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			if (col == 1 && row == 1)
				continue;

			live_cells += submatrix[row][col]; // 8 leituras 8 escritas 8 FLOPS 
		}
	}

	return (cell == 1 && (live_cells >= rules[1] && live_cells <= rules[0])) ||
		   (cell == 0 && live_cells == rules[2]); // 3 leituras
}

// NEW Set Cell
void set_cell(int **world, size_t row, size_t col, int value)
{
    world[row][col] = value; // 1 Escrita
}

// NEW Get Cell
int get_cell(int **world, size_t row, size_t col)
{
    return world[row][col]; // 1 Leitura
}

// NEW Copy World
void copy_world(int **world1, size_t rows_count, size_t cols_count, int **world2)
{
    #pragma omp parallel for
    for (size_t row = 1; row <= rows_count; row++)
    {
        for (size_t col = 1; col <= cols_count; col++)
        {
            world1[row][col] = world2[row][col]; // N² escritas + N² Leituras
        }
    }
}

// NEW Update World
void update_world(int **world, size_t rows_count, size_t cols_count, int **world_aux, const int rules[RULE_SIZE])
{
    #pragma omp parallel for
    for (size_t row = 1; row <= rows_count; row++)
    {
        for (size_t col = 1; col <= cols_count; col++)
        {
            int pre_row = row - 1; // 1FLOP, 1 Escrita
			if (pre_row == 0)
				pre_row = rows_count;

			int pos_row = row + 1; // 1FLOP, 1 Escrita
			if (pos_row == rows_count + 1)
				pos_row = 1;

			int pre_col = col - 1; // 1FLOP, 1 Escrita
			if (pre_col == 0)
				pre_col = cols_count;

			int pos_col = col + 1; // 1FLOP, 1 Escrita
			if (pos_col == cols_count + 1)
				pos_col = 1;

			int submatrix[3][3];  //18 acessos, 9 escritas, 9 leituras
			submatrix[0][0] = get_cell(world, pre_row, pre_col);
			submatrix[0][1] = get_cell(world, pre_row, col);
			submatrix[0][2] = get_cell(world, pre_row, pos_col);
			submatrix[1][0] = get_cell(world, row, pre_col);
			submatrix[1][1] = get_cell(world, row, col);
			submatrix[1][2] = get_cell(world, row, pos_col);
			submatrix[2][0] = get_cell(world, pos_row, pre_col);
			submatrix[2][1] = get_cell(world, pos_row, col);
			submatrix[2][2] = get_cell(world, pos_row, pos_col);

			set_cell(world_aux, row, col, cell_lives(submatrix, rules)); // 1 Escrita + Cell_lives
        }
    }

    copy_world(world, rows_count, cols_count, world_aux);
}

// NEW Update World N Generations
void update_world_n_generations(size_t n, int **world, size_t rows_count, size_t cols_count, int **world_aux, const int rules[RULE_SIZE])
{
    if (n <= 0)
        return;

    for (int i = 0; i < n; i++)
        update_world(world, rows_count, cols_count, world_aux, rules);
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

    for (size_t i = 0; i < n; i++)
    {
        world[i] = malloc(n * sizeof(int));

        if (world[i] == NULL)
        {
            fprintf(stderr, "Failed to alloc WORLD!");
            exit(1);
        }
    }

    if (fill)
    {
        srand(SEED);

        for (size_t i = 1; i <= size; i++)
        {
            for (size_t j = 1; j <= size; j++)
            {
                world[i][j] = rand() & 1;
            }
        }
    }

    return world;
}

void free_world(int **world, size_t size)
{
    if (!world)
        return;

    for (size_t i = 0; i < size+2; i++)
        free(world[i]);

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
    
    free_world(world, cfg.size);

    puts("\n***SUCESS***\n");
    printf("World Size = %dx%d\n", cfg.size, cfg.size);
    printf("Generations = %d\n", cfg.generations);
    printf("Runtime = %.6lf seconds\n", exec_time);
    return 0;
}