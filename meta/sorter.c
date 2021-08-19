/**
 * This utility sorts things out
 *
 * Finds common microcommands, groups them together and moves them around
 * so that all emulated chips have same microcommand numbers
 */

#include <stdio.h>
#include <string.h>

#define SYNCOUNT         128
#define UCMDCOUNT        59  // up to this index we can freely move uCs

// skip inclusion of AVR header
#define _OPT_GENER
// temporarily overwrite some macros
#define PROGMEM
#define const
#include "../roms_original.h"
#undef const


typedef struct scblock
{
    UCmd28 *ucmds;
    Synch  *synch;
    char   name[10];
    int    map[UCMDCOUNT];
}
scblock_t;


int get_ucmd_index(UCmd28 *arr, UCmd28 c)
{
    for (int i = 0; i < UCMDCOUNT; i++)
    {
        if (memcmp(&arr[i], &c, sizeof(UCmd28)) == 0)
            return i;
    }
    return -1;
}

int is_common_ucmd(UCmd28 c)
{
    return (get_ucmd_index(ucmds_1302, c) >= 0)
        && (get_ucmd_index(ucmds_1303, c) >= 0)
        && (get_ucmd_index(ucmds_1306, c) >= 0);
}

int is_established_ucmd(UCmd28 c)
{
    return (get_ucmd_index(ucmds_1302, c) == get_ucmd_index(ucmds_1303, c))
        && (get_ucmd_index(ucmds_1303, c) == get_ucmd_index(ucmds_1306, c));
}

void swap_ucmd_places_synch(Synch *arr, int ix_a, int ix_b)
{
    printf("UPD SYN:");
    for (int i = 0; i < SYNCOUNT; i++)
    {
        Synch new;
        int ctr = 0;
        for (int j = 0; j < sizeof(new.byte); j++)
        {
            if      (arr[i].byte[j] == ix_a) { new.byte[j] = ix_b; ctr++; }
            else if (arr[i].byte[j] == ix_b) { new.byte[j] = ix_a; ctr++; }
            else                             new.byte[j] = arr[i].byte[j];
        }
        arr[i] = new;
        if (ctr) printf(" #%d(%d)", i, ctr);
    }
    printf("\n");
}

void swap_ucmd_places_ucmds(UCmd28 *arr, int ix_a, int ix_b)
{
    UCmd28 temp;
    printf("SWAP UC %d <-> %d\n", ix_a, ix_b);
    temp = arr[ix_a];
    arr[ix_a] = arr[ix_b];
    arr[ix_b] = temp;
}

void swap_ucmd_places(scblock_t *blk, int ix_a, int ix_b)
{
    int temp = blk->map[ix_a];
    blk->map[ix_a] = blk->map[ix_b];
    blk->map[ix_b] = temp;
    swap_ucmd_places_ucmds (blk->ucmds, ix_a, ix_b);
    swap_ucmd_places_synch (blk->synch, ix_a, ix_b);
}

void build_common_ucmds_map(scblock_t *blk)
{
    printf("--- Building map for \"%s\"\n", blk->name);

    for (int i = 0; i < UCMDCOUNT; i++)
    {
        if (is_established_ucmd (blk->ucmds[i]))
        {
            printf("%d ESTABLISHED\n", i);
            blk->map[i] = 0;
            continue;
        }
        if (is_common_ucmd (blk->ucmds[i]))
        {
            printf("%d COMMON\n", i);
            blk->map[i] = 1;
            continue;
        }
        printf("%d UNIQUE\n", i);
        blk->map[i] = -1;
    }
}

void group_common_ucmds(scblock_t *blk)
{
    printf("--- Grouping common microcommands for \"%s\"\n", blk->name);

    for (int i = 0; i < UCMDCOUNT; i++)
    {
        if (blk->map[i] > 0)
        {
            // find first unique ucmd below the common one
            for (int j = 0; j < UCMDCOUNT; j++)
            {
                // if there is, swap them places
                if (blk->map[j] < 0)
                {
                    swap_ucmd_places (blk, i, j);
                    break;
                }
            }
        }
    }
}

int compare_ucmds(UCmd28 a, UCmd28 b)
{
    return memcmp(&a, &b, sizeof(UCmd28));
}

void establish_common_ucmds(scblock_t *blk)
{
    printf("--- Sorting microcommands for \"%s\"\n", blk->name);

    // find first common item
    int fci = 0;
    for (; blk->map[fci] <= 0; fci++);

    // find last common item
    int lci = 0;
    for (lci = fci; blk->map[lci] > 0; lci++); lci--;

    printf ("--- Range %d..%d\n", fci, lci);
    // sort 'em
    for (int i = fci; i <= lci - 1; i++)
    {
        for (int j = i; j <= lci; j++)
        {
            if (compare_ucmds(blk->ucmds[i], blk->ucmds[j]) > 0)
            {
                swap_ucmd_places(blk, i, j);
            }
        }
    }
}

void dump_synch(FILE *f, Synch s)
{
    for (int j = 0; j < sizeof(s.byte); j++)
    {
        fprintf(f, "%s0x%02X", (j != 0 ? ", " : ""), s.byte[j]);
    }
}

void dump_ucmd(FILE *f, UCmd28 c)
{
    for (int j = 0; j < sizeof(c.byte); j++)
    {
        fprintf(f, "%s0x%02X", (j != 0 ? ", " : ""), c.byte[j]);
    }
}

void dump_rom_for(scblock_t *blk)
{
    FILE *f = fopen ("roms.h", "a");

    fprintf(f, "const Synch syn_%s[%d] PROGMEM = {\n", blk->name, SYNCOUNT);
    for (int i = 0; i < SYNCOUNT; i++)
    {
        fprintf(f, "    {");
        dump_synch(f, blk->synch[i]);
        fprintf(f, "}%c\n", i != SYNCOUNT-1 ? ',' : ' ');
    }
    fprintf(f, "};\n\n");

    fprintf(f, "const UCmd28 ucmds_%s[%d] PROGMEM = {\n", blk->name, SYNCOUNT);
    for (int i = 0; i < UCMDCOUNT + 9; i++)
    {
        fprintf(f, "    {");
        dump_ucmd(f, blk->ucmds[i]);
        fprintf(f, "}%c\n", i != SYNCOUNT-1 ? ',' : ' ');
    }
    fprintf(f, "};\n\n");

    fclose (f);
}

void print_established(scblock_t *blk)
{
    printf ("\n--- Established common microcommands:\n");
    for (int i = 0; i < UCMDCOUNT; i++)
    {
        if (is_established_ucmd(blk->ucmds[i]))
        {
            printf("%03d   %08X\n", i, *((unsigned*) &blk->ucmds[i]));
        }
    }
}


int main()
{
    scblock_t blk[3] = {
        { ucmds_1302, syn_1302, "1302" },
        { ucmds_1303, syn_1303, "1303" },
        { ucmds_1306, syn_1306, "1306" },
    };

    for (int i = 0; i < 3; i++)
    {
        build_common_ucmds_map (&blk[i]);
        group_common_ucmds(&blk[i]);
        establish_common_ucmds(&blk[i]);
        dump_rom_for(&blk[i]);
    }

    print_established (&blk[0]);

    return 0;
}
