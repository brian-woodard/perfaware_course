#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>

#include "listing_0065_haversine_formula.cpp"

struct Pair
{
   f64 x0;
   f64 y0;
   f64 x1;
   f64 y1;
};

union PairUnion
{
   Pair pts;
   f64  array[4];
};


bool parse_json(const char* Filename, std::vector<PairUnion>& pairs)
{
   FILE*     fd = fopen(Filename, "r");
   PairUnion pair;
   int       pair_idx = 0;

   if (fd)
   {
      char line[256];

      // skip first 2 lines
      fscanf(fd, "%s", line);
      fscanf(fd, "%s", line);

      // assume a pair per line, until ending ']' is read
      while (!feof(fd))
      {
         int  index = 0;
         int  start_idx = 0;
         int  end_idx = 0;
         f64  value = 0.0;
         char value_str[25] = {};

         fscanf(fd, "%s", line);

         while (1)
         {
            if (line[index] == ':')
               start_idx = index + 1;

            if (line[index] == ',' ||
                line[index] == '}' ||
                line[index] == '\0')
            {
               end_idx = index;
               break;
            }

            if (line[index] == ']')
               return true;

            index++;
         }

         memcpy(value_str, &line[start_idx], end_idx - start_idx);

         value = atof(value_str);

         pair.array[(pair_idx%4)] = value;
         pair_idx++;

         if (pair_idx % 4 == 0)
            pairs.push_back(pair);
      }

      fclose(fd);
   }
   else
      return false;

   return true;
}

int main(int argc, char* argv[])
{
   std::vector<PairUnion> pairs;
   f64*                   haversine_distances = nullptr;
   f64                    total = 0.0;

   if (argc != 2)
   {
      printf("haversine_processor [json input]\n");
      exit(1);
   }

   if (!parse_json(argv[1], pairs))
   {
      printf("Failed to parse %s\n", argv[1]);
      exit(1);
   }

   printf("Read %ld pairs\n", pairs.size());

   haversine_distances = new f64[pairs.size()];

   for (size_t i = 0; i < pairs.size(); i++)
   {
      haversine_distances[i] = ReferenceHaversine(pairs[i].pts.x0, pairs[i].pts.y0,
                                                  pairs[i].pts.x1, pairs[i].pts.y1,
                                                  6372.8);

      total += haversine_distances[i];
   }

   total = total / pairs.size();

   printf("Distance: %.5f\n", total);

}
