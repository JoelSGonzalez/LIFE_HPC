#include "life.h"

int cell_lives(const int submatrix[3][3], const int rule[RULE_SIZE])
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

	return (cell == 1 && (live_cells >= rule[1] && live_cells <= rule[0])) ||
		   (cell == 0 && live_cells == rule[2]); // 3 leituras
}

// OLD Clear World
// void clear_world(int world[][MAX_COLS], int rows_count, int cols_count)
// {
// 	if (cols_count > VIRTUAL_MAX_COLS || rows_count < 2 || cols_count < 2)
// 		return;
// 	for (size_t row = 1; row < rows_count + 1; row++)
// 	{
// 		for (size_t col = 1; col < cols_count + 1; col++)
// 		{
// 			world[row][col] = 0;
// 		}
// 	}
// }

// NEW Clear World
void clear_world(int **world, size_t rows_count, size_t cols_count)
{
    for (size_t row = 0; row < rows_count + 2; row++)
    {
        for (size_t col = 0; col < cols_count + 2; col++)
        {
            world[row][col] = 0; // rows*cols escritas
        }
    }
}

// OLD Set Cell
// void set_cell(int world[][MAX_COLS], int row, int col, int value)
// {
// 	world[row][col] = value;
// }

// NEW Set Cell
void set_cell(int **world, size_t row, size_t col, int value)
{
    world[row][col] = value; // 1 Escrita
}

// OLD Get Cell 
// int get_cell(const int world[][MAX_COLS], int row, int col)
// {
// 	return world[row][col];
// }

// NEW Get Cell
int get_cell(const int **world, size_t row, size_t col)
{
    return world[row][col]; // 1 Leitura
}

// OLD copy_world
// void copy_world(
// 	int world1[][MAX_COLS], int rows_count, int cols_count,
// 	int world2[][MAX_COLS])
// {
// 	for (size_t row = 1; row < rows_count + 1; row++)
// 	{
// 		for (size_t col = 1; col < cols_count + 1; col++)
// 		{
// 			world1[row][col] = world2[row][col];
// 		}
// 	}
// }

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

// OLD Update World
// void update_world(
// 	int world[][MAX_COLS],
// 	int rows_count, int cols_count,
// 	int world_aux[][MAX_COLS], const int rule[RULE_SIZE])
// {
// 	for (size_t row = 1; row < rows_count + 1; row++)
// 	{
// 		for (size_t col = 1; col < cols_count + 1; col++)
// 		{
// 			int pre_row = row - 1; // 1FLOP, 1 Escrita
// 			if (pre_row == 0)
// 				pre_row = rows_count;
//
// 			int pos_row = row + 1; // 1FLOP, 1 Escrita
// 			if (pos_row == rows_count + 1)
// 				pos_row = 1;
//
// 			int pre_col = col - 1; // 1FLOP, 1 Escrita
// 			if (pre_col == 0)
// 				pre_col = cols_count;
//
// 			int pos_col = col + 1; // 1FLOP, 1 Escrita
// 			if (pos_col == cols_count + 1)
// 				pos_col = 1;
//
// 			int submatrix[3][3];  //18 acessos, 9 escritas, 9 leituras
// 			submatrix[0][0] = get_cell(world, pre_row, pre_col);
// 			submatrix[0][1] = get_cell(world, pre_row, col);
// 			submatrix[0][2] = get_cell(world, pre_row, pos_col);
// 			submatrix[1][0] = get_cell(world, row, pre_col);
// 			submatrix[1][1] = get_cell(world, row, col);
// 			submatrix[1][2] = get_cell(world, row, pos_col);
// 			submatrix[2][0] = get_cell(world, pos_row, pre_col);
// 			submatrix[2][1] = get_cell(world, pos_row, col);
// 			submatrix[2][2] = get_cell(world, pos_row, pos_col);
//
// 			set_cell(world_aux, row, col, cell_lives(submatrix, rule));
// 		}
// 	}
//
// 	copy_world(world, rows_count, cols_count, world_aux);
// }

// NEW Update World
void update_world(int **world, size_t rows_count, size_t cols_count, int **world_aux, const int rule[RULE_SIZE])
{
    for (size_t row = 1; row <= rows_count; row++)
    {
        for (size_t col = 1; col <= cols_count; col++)
        {
            int submatrix[3][3];

            for (int i = -1; i <= 1; i++)
            {
				size_t r = (row + i - 1 + rows_count) % rows_count + 1; // 3*(1 Escritas 5 Flops)*(N²)
                for (int j = -1; j <= 1; j++)
                {
                    size_t c = (col + j - 1 + cols_count) % cols_count + 1; // 9*(1 Escrita  5 Flops)*(N²)

                    submatrix[i + 1][j + 1] = world[r][c];
                }
            }

            world_aux[row][col] = cell_lives(submatrix, rule);
        }
    }

    copy_world(world, rows_count, cols_count, world_aux);
}

// OLD Update World N Generations
// void update_world_n_generations(
// 	int n, int world[][MAX_COLS],
// 	int rows_count, int cols_count,
// 	int world_aux[][MAX_COLS],
// 	const int rule[RULE_SIZE])
// {
// 	if (n <= 0)
// 		return;
//
// 	for (size_t i = 0; i <= n; i++)
// 	{
// 		update_world(world, rows_count, cols_count, world_aux, rule);
// 	}
// }

// NEW Update World N Generations
void update_world_n_generations(size_t n, int **world, size_t rows_count, size_t cols_count, int **world_aux, const int rule[RULE_SIZE])
{
    if (n <= 0)
        return;

    for (int i = 0; i < n; i++)
        update_world(world, rows_count, cols_count, world_aux, rule);
}

// OLD File Print World
// void fprint_world(const int world[][MAX_COLS], int rows_count, int cols_count, FILE *__restrict__ __stream)
// {
// 	for (size_t row = 1; row < rows_count + 1; row++)
// 	{
// 		for (size_t col = 1; col < cols_count + 1; col++)
// 		{
// 			fputs((world[row][col]) ? "X " : ". ", __stream);
// 		}
// 		fputc('\n', __stream);
// 	}
// }

// NEW File Print World
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

// OLD Print World
// void print_world(const int world[][MAX_COLS], int rows_count, int cols_count)
// {
// 	fprint_world(world, rows_count, cols_count, stdout);
// }

// NEW Print World
void print_world(int **world, size_t rows_count, size_t cols_count)
{
    fprint_world(world, rows_count, cols_count, stdout);
}

// OLD Write World
// void write_world(const int world[][MAX_COLS], int rows_count, int cols_count, const char *filename)
// {
// 	FILE *file = fopen(filename, "w");
// 	if (file == NULL)
// 	{
// 		printf("Error!\n");
// 		exit(-1);
// 	}
//
// 	fprint_world(world, rows_count, cols_count, file);
//
// 	fclose(file);
// }

// NEW Write WOrld
void write_world(int **world, size_t rows_count, size_t cols_count, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    fprint_world(world, rows_count, cols_count, file);

    fclose(file);
}

// OLD Read World
// void read_world(int world[FILE_MAX_LINES + 2][MAX_COLS], int world_size[2], const char *filename)
// {
// 	FILE *file = fopen(filename, "r");
// 	if (file == NULL)
// 	{
// 		printf("Error!\n");
// 		exit(1);
// 	}
//
// 	int row = 1;
// 	int col = 1;
//
// 	for (char c = getc(file); c != EOF; c = getc(file))
// 	{
// 		switch (c)
// 		{
// 		case '\n':
// 			if (row == 1)
// 				world_size[1] = col - 1;
//
// 			row++;
// 			col = 1;
// 			break;
// 		case '.':
// 		case 'X':
// 			set_cell(world, row, col++, c == 'X');
// 			break;
// 		default:
// 			break;
// 		}
// 	}
//
// 	world_size[0] = row - 1;
//
// 	fclose(file);
// }

// NEW Read World
void read_world(int **world, size_t rows_count, size_t cols_count, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    size_t row = 1;
    size_t col = 1;

    int c;

    while ((c = getc(file)) != EOF)
    {
        switch (c)
        {
        case '\n':
            row++;
            col = 1;
            break;

        case '.':
            world[row][col++] = 0;
            break;

        case 'X':
            world[row][col++] = 1;
            break;

        default:
            break;
        }
    }

    fclose(file);
}