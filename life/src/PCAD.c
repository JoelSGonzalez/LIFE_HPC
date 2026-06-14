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

int **init_world(size_t size)
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


    int **world = init_world(cfg.size);
    if (world != NULL) puts("Inicializou!");

    if(cfg.size <= 100)
    {
        printf("Coisa boa!\n");
    }

    free_world(world);

    puts("TUDO CERTO");
    return 0;
}