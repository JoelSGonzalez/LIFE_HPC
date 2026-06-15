#include "life.h"

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
    Config cfg = {0, 0};
    int opt;

    opterr = 0;

    while ((opt = getopt(argc, argv, ":s:g:")) != -1)
    {
        switch (opt)
        {
            case 's':
                cfg.size = parse_uint(optarg);
                break;

            case 'g':
                cfg.generations = parse_uint(optarg);
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
    
    start_time = omp_get_wtime();
    update_world_n_generations(cfg.generations, world, cfg.size, cfg.size, aux, base_rules);
    finish_time = omp_get_wtime();
    exec_time = finish_time - start_time;
    
    free_world(world);

    puts("\n***SUCESS***\n");
    printf("World Size = %dx%d\n", cfg.size, cfg.size);
    printf("Generations = %d\n", cfg.generations);
    printf("Runtime = %.6lf seconds\n", exec_time);
    return 0;
}