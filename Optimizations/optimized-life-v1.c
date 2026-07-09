/* Optimized-Life-V1.c
    Otimização comparada ao baseline:
        - Alocação de memória contígua (Matriz de 1 dimensão);
        - Criação de macro para percorrer a matriz de 1 dimensão;
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <omp.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include <papi.h>

#define RULE_SIZE 3
#define VIRTUAL_MAX_COLS 71
#define MAX_COLS VIRTUAL_MAX_COLS + 2
#define FILE_MAX_LINES 27

#define MIN_GRID_SIZE 3
#define MIN_GEN 1
#define SEED 25032002

// Macro para percorrer a matriz de mundo.
#define IDX(row, col, dim) ((row) * (dim) + (col))

typedef struct
{
    size_t size;
    size_t generations;
	int print;
} Config;


//==========================================================
// Validador de entrada por linha de comando
//  Não faz parte do algoritmo
//==========================================================
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
//==========================================================
// Fim das funções do validador
//==========================================================

// NEW Update World
//====================================
// update_world otimizado
// Calcula os vizinhos diretamente na matriz do mundo
// A atualização ocorre diretamente no mundo auxiliar
// Set_cell(), Get_cell() e Cell_lives() ficam obseletos com a mudança.
//====================================
// Update one generation using double buffering (no copy!)
void update_world(int **world_ptr, int **aux_ptr, size_t rows_count,
                  size_t cols_count, const int rules[RULE_SIZE]){

    int *world = *world_ptr;
    int *aux   = *aux_ptr;
    int dim    = (int)(rows_count + 2);   // Total de elementos em uma linha

    #pragma omp parallel for schedule(runtime)
    for (size_t row = 1; row <= rows_count; row++) {
        for (size_t col = 1; col <= cols_count; col++) {
            int pre_row, pos_row, pre_col, pos_col;
            
            // Lógica de wrap around
            if(row == 1) {
                pre_row = (int)rows_count;
            } else {
                pre_row = (int)(row - 1);
            }
            
            if(row == rows_count) {
                pos_row = 1;
            } else {
                pos_row = (int)(row + 1);
            }
            
            // Wrap columns
            if(col == 1) {
                pre_col = (int)cols_count;
            } else {
                pre_col = (int)(col - 1);
            }
            
            if(col == cols_count) {
                pos_col = 1;
            } else {
                pos_col = (int)(col + 1);
            }
            
            // ATUALIZAÇÃO IMPORTANTE: NÃO CRIA NOVA MATRIZ
            // Soma dos 8 vizinhos diretamente do vetor
            // (pre_row * dim) + pre_col
            int live_neighbors =
                world[IDX(pre_row, pre_col, dim)] +
                world[IDX(pre_row, col, dim)] +
                world[IDX(pre_row, pos_col, dim)] +
                world[IDX(row, pre_col, dim)] +
                // skip center cell
                world[IDX(row, pos_col, dim)] +
                world[IDX(pos_row, pre_col, dim)] +
                world[IDX(pos_row, col, dim)] +
                world[IDX(pos_row, pos_col, dim)];

            int cell = world[IDX(row, col, dim)];
            int new_state = (cell == 1 && live_neighbors >= rules[1] &&
                             live_neighbors <= rules[0]) ||
                            (cell == 0 && live_neighbors == rules[2]);

            aux[IDX(row, col, dim)] = new_state;
        }
    }

    // ATUALIZAÇÃO IMPORTANTE: SWAP DE BUFFERS
    // Troca de buffer, o mundo aponta diretamente para a nova geração
    int *tmp = *world_ptr;
    *world_ptr = *aux_ptr;
    *aux_ptr = tmp;
}

void update_world_n_generations(size_t n, int **world, int **aux,
                                size_t rows_count, size_t cols_count,
                                const int rules[RULE_SIZE]) {
    for(size_t i = 0; i < n; i++)
        update_world(world, aux, rows_count, cols_count, rules);
}

// ATUALIZAÇÃO IMPORTANTE: MATRIZ 1D CONTÍGUA
int *init_world(size_t size, int fill){
    size_t dim = size + 2;               // Incluí o "halo"
    int *world = calloc(dim * dim, sizeof(int)); // Alocação contígua de vetor 
    if(!world){
        fprintf(stderr, "Failed to alloc WORLD!\n");
        exit(1);
    }

    if(fill){
        srand(SEED);
        for (size_t i = 1; i <= size; i++)
            for (size_t j = 1; j <= size; j++)
                world[IDX(i, j, dim)] = rand() & 1; // Preenche o mundo com 0s e 1s
    }
    return world;
}

void free_world(int *world){ // Mudança de ponteiro da função
    free(world);
}

int main(int argc, char **argv)
{
    Config cfg = get_config(argc, argv);
    const int base_rules[RULE_SIZE] = {3,2,3};

    int *world = init_world(cfg.size, 1); // Altera para 1_ptr
    int *aux = init_world(cfg.size, 0); // Altera para 1_ptr

    double start_time, finish_time, exec_time;

    // PAPI Inicializadores
    int EventSet = PAPI_NULL;
    long long values[2];

    // Inicia biblioteca
    if(PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        fprintf(stderr, "Erro ao inicializar o PAPI\n");
        exit(1);
    }

    // Cria observadores de eventos
    PAPI_create_eventset(&EventSet);
    PAPI_add_event(EventSet, PAPI_L1_DCM);
    PAPI_add_event(EventSet, PAPI_L2_DCM);
    
    if(world == NULL||aux==NULL){
        fprintf(stderr, "Failed to initialize\n");
        exit(1);
    }
    
    
    if(!cfg.print){
        // Inicia os observadores
        PAPI_start(EventSet);

        start_time = omp_get_wtime();
        // Atualização: Parametros da função atualizada
        update_world_n_generations(cfg.generations, &world, &aux,
                                   cfg.size, cfg.size, base_rules);   
        finish_time = omp_get_wtime();

        // Encerra os observadores
        PAPI_stop(EventSet, values);
        
        exec_time = finish_time - start_time;
    }

    uint64_t checksum = 0;
    int dim = (int)(cfg.size + 2);
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            checksum += *(uint32_t*)&world[IDX(i, j, dim)];
        }
    }

    free_world(world); // Só 1 parametro
    free_world(aux);

    // puts("\n***SUCESS***\n");
    // printf("World Size = %dx%d\n", cfg.size, cfg.size);
    // printf("Generations = %d\n", cfg.generations);
    printf("%.6lf\n", exec_time);
    printf("%ld\n", checksum);
    printf("L1 misses: %lld, L2 misses: %lld\n", values[0], values[1]);
    return 0;
}