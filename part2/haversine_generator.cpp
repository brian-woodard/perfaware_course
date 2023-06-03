#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#include "listing_0065_haversine_formula.cpp"

struct Location
{
   f64 lat;
   f64 lon;
};

struct Cluster
{
   f64 lat;
   f64 lon;
   f64 lat_radius;
   f64 lon_radius;
};

const int NUM_CLUSTERS = 8;

f64 GenerateRandomNumber()
{
   return (f64)rand() / (f64)RAND_MAX;
}

void GeneratePoint(Location* Point, bool Latitude, Cluster* ClusterInfo = nullptr)
{
   f64 rand_f = GenerateRandomNumber();

   if (!ClusterInfo)
   {
      if (Latitude)
         Point->lat = (rand_f - 0.5) * 180.0;
      else
         Point->lon = (rand_f - 0.5) * 360.0;
   }
   else
   {
      if (Latitude)
         Point->lat = ((rand_f - 0.5) * ClusterInfo->lat_radius) + ClusterInfo->lat;
      else
         Point->lon = ((rand_f - 0.5) * ClusterInfo->lon_radius) + ClusterInfo->lon;
   }
}

int main(int argc, char* argv[])
{
   int       number_of_pairs = 0;
   int       seed = 0;
   bool      uniform = true;
   f64*      haversine_distances = nullptr;
   Location* haversine_pairs = nullptr;
   Cluster*  cluster_info = nullptr;
   f64       total = 0;
   FILE*     fd = nullptr;

   if (argc != 4)
   {
      printf("haversine_generator [uniform | cluster] [seed] [number of pairs]\n");
      exit(1);
   }

   if (strcmp("cluster", argv[1]) == 0)
      uniform = false;

   seed = atoi(argv[2]);
   number_of_pairs = atoi(argv[3]);

   fd = fopen("haversine_points.json", "w");
   if (!fd)
   {
      fprintf(stderr, "Error opening file: haversine_points.json\n");
      exit(1);
   }

   srand(seed);

   printf("Generating %d pairs...\n", number_of_pairs);

   haversine_distances = new f64[number_of_pairs];
   haversine_pairs = new Location[number_of_pairs*2];

   if (!uniform)
   {
      cluster_info = new Cluster[NUM_CLUSTERS];
      for (int i = 0; i < NUM_CLUSTERS; i++)
      {
         // generate radius from 15.0 - 40 degrees
         cluster_info[i].lat_radius = (GenerateRandomNumber() * 30.0) + 10.0;
         cluster_info[i].lon_radius = (GenerateRandomNumber() * 30.0) + 10.0;

         // generate starting point
         cluster_info[i].lat = (GenerateRandomNumber() - 0.5) * 180.0;
         cluster_info[i].lon = (GenerateRandomNumber() - 0.5) * 360.0;

         if (cluster_info[i].lat > 0.0 && cluster_info[i].lat < cluster_info[i].lat_radius)
            cluster_info[i].lat += cluster_info[i].lat_radius;
         else if (cluster_info[i].lat < 90.0 && cluster_info[i].lat > 90.0 - cluster_info[i].lat_radius)
            cluster_info[i].lat -= cluster_info[i].lat_radius;
         else if (cluster_info[i].lat < 0.0 && cluster_info[i].lat > -cluster_info[i].lat_radius)
            cluster_info[i].lat -= cluster_info[i].lat_radius;
         else if (cluster_info[i].lat > -90.0 && cluster_info[i].lat < -90.0 + cluster_info[i].lat_radius)
            cluster_info[i].lat += cluster_info[i].lat_radius;

         if (cluster_info[i].lon > 0.0 && cluster_info[i].lon < cluster_info[i].lon_radius)
            cluster_info[i].lon += cluster_info[i].lon_radius;
         else if (cluster_info[i].lon < 180.0 && cluster_info[i].lon > 180.0 - cluster_info[i].lon_radius)
            cluster_info[i].lon -= cluster_info[i].lon_radius;
         else if (cluster_info[i].lon < 0.0 && cluster_info[i].lon > -cluster_info[i].lon_radius)
            cluster_info[i].lon -= cluster_info[i].lon_radius;
         else if (cluster_info[i].lon > -180.0 && cluster_info[i].lon < -180.0 + cluster_info[i].lon_radius)
            cluster_info[i].lon += cluster_info[i].lon_radius;
      }
   }

   fprintf(fd, "{\"pairs\":\n");
   fprintf(fd, "   [\n");

   for (int i = 0; i < number_of_pairs; i++)
   {
      int idx1 = i*2;
      int idx2 = idx1+1;
      int idxc = i % NUM_CLUSTERS;

      if (uniform)
      {
         GeneratePoint(&haversine_pairs[idx1], true);
         GeneratePoint(&haversine_pairs[idx1], false);
         GeneratePoint(&haversine_pairs[idx2], true);
         GeneratePoint(&haversine_pairs[idx2], false);
      }
      else
      {
         GeneratePoint(&haversine_pairs[idx1], true, &cluster_info[idxc]);
         GeneratePoint(&haversine_pairs[idx1], false, &cluster_info[idxc]);
         GeneratePoint(&haversine_pairs[idx2], true, &cluster_info[idxc]);
         GeneratePoint(&haversine_pairs[idx2], false, &cluster_info[idxc]);
      }

      haversine_distances[i] = ReferenceHaversine(haversine_pairs[idx1].lon, haversine_pairs[idx1].lat,
                                                  haversine_pairs[idx2].lon, haversine_pairs[idx2].lat,
                                                  6372.8);

      if (i == number_of_pairs-1)
      {
         fprintf(fd, "      {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}\n",
            haversine_pairs[idx1].lon, haversine_pairs[idx1].lat,
            haversine_pairs[idx2].lon, haversine_pairs[idx2].lat);
      }
      else
      {
         fprintf(fd, "      {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f},\n",
            haversine_pairs[idx1].lon, haversine_pairs[idx1].lat,
            haversine_pairs[idx2].lon, haversine_pairs[idx2].lat);
      }

      total += haversine_distances[i];
   }

   total = total / number_of_pairs;

   fprintf(fd, "   ]\n");
   fprintf(fd, "}\n");

   printf("Type: %s\n", argv[1]);
   printf("Seed: %d\n", seed);
   printf("Number of pairs: %d\n", number_of_pairs);
   printf("Distance: %.5f\n", total);

   fclose(fd);

   // write out haversine_distances to file
   fd = fopen("haversine_distances.bin", "wb");
   for (int i = 0; i < number_of_pairs; i++)
      fwrite(&haversine_distances[i], sizeof(f64), 1, fd);

   fwrite(&total, sizeof(f64), 1, fd);

   fclose(fd);

   delete [] haversine_distances;
   delete [] haversine_pairs;
}
