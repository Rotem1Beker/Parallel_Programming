// Rotem Beker 217386598

#include "kmeans.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

PointSet *createPointSet(int numPoints) {
    PointSet *p = (PointSet *)malloc(sizeof(PointSet));
    if (p == NULL) return NULL;
    p->numPoints = numPoints;
    p->points = (Point *)malloc((size_t)numPoints * sizeof(Point));
    p->assignments = (int *)malloc((size_t)numPoints * sizeof(int));
    for (int i = 0; i < numPoints; i++) {
        p->assignments[i] = 0;
    }
    return p;
}

void freePointSet(PointSet *p) {
    if (p == NULL) return;
    free(p->points);
    free(p->assignments);
    free(p);
}

Centroids *createCentroids(int k) {
    Centroids *c = (Centroids *)malloc(sizeof(Centroids));
    if (c == NULL) return NULL;
    c->k = k;
    c->centroids = (Point *)malloc((size_t)k * sizeof(Point));
    return c;
}

void freeCentroids(Centroids *c) {
    if (c == NULL) return;
    free(c->centroids);
    free(c);
}

static inline double squaredDistance(Point a, Point b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

int runKMeans(PointSet *data, Centroids *centroids, int maxIters, double tolerance) {
    int n = data->numPoints;
    int k = centroids->k;
    double tolSquared = tolerance * tolerance;

    // Standard local variables prevent data cross-contamination between benchmark passes
    int current_iter = 0; 
    int done = 0;

    double *sumX = (double *)calloc((size_t)k, sizeof(double));
    double *sumY = (double *)calloc((size_t)k, sizeof(double));
    int *counts  = (int *)calloc((size_t)k, sizeof(int));

    #pragma omp parallel shared(done, current_iter)
    {
        while (1) {
            #pragma omp single
            {
                if (done || current_iter >= maxIters) {
                    done = 1;
                } else {
                    current_iter++;
                    for (int c = 0; c < k; c++) {
                        sumX[c] = 0.0;
                        sumY[c] = 0.0;
                        counts[c] = 0;
                    }
                }
            }
            /* implicit barrier */

            if (done) break;

            /* Phase 2: Assignment */
            #pragma omp for schedule(static)
            for (int i = 0; i < n; i++) {
                double minDist = DBL_MAX;
                int bestCluster = 0;
                for (int c = 0; c < k; c++) {
                    double dist = squaredDistance(data->points[i], centroids->centroids[c]);
                    if (dist < minDist) {
                        minDist = dist;
                        bestCluster = c;
                    }
                }
                data->assignments[i] = bestCluster;
            }
            /* implicit barrier */

            /* Phase 3: Accumulation using OpenMP array reduction */
            #pragma omp for schedule(static) \
                reduction(+: sumX[:k], sumY[:k], counts[:k])
            for (int i = 0; i < n; i++) {
                int c = data->assignments[i];
                sumX[c]   += data->points[i].x;
                sumY[c]   += data->points[i].y;
                counts[c] += 1;
            }
            /* implicit barrier */

            /* Phase 4: Update centroids + convergence check */
            #pragma omp single
            {
                double maxMovement = 0.0;
                for (int c = 0; c < k; c++) {
                    if (counts[c] == 0) continue;
                    Point updated;
                    updated.x = sumX[c] / counts[c];
                    updated.y = sumY[c] / counts[c];
                    double mv = squaredDistance(centroids->centroids[c], updated);
                    if (mv > maxMovement) maxMovement = mv;
                    centroids->centroids[c] = updated;
                }

                if (maxMovement < tolSquared) {
                    done = 1; 
                }
            }
            /* implicit barrier */
        }
    } /* end parallel region */

    free(sumX);
    free(sumY);
    free(counts);

    return current_iter;
}