#include <brender.h>
#include <cstdio>

char buf[1024 * 1024 * 1024]; /* 1GiB */

typedef struct Graph {
    float from;
    float to;
    float step;
    float (*eval_float)(float x);
    float (*eval_fixed)(float x);
} Graph;

const Graph graph_sin = {
    .from       = -10.0f,
    .to         = 10.0f,
    .step       = 0.001f,
    .eval_float = [](float x) -> float { return sinf(x * 2.0f * M_PI); },
    .eval_fixed = [](float x) -> float { return BrFixedToFloat(BrFixedSin(BrFloatToFixed(x))); },
};

const Graph graph_cos = {
    .from       = -10.0f,
    .to         = 10.0f,
    .step       = 0.001f,
    .eval_float = [](float x) -> float { return cosf(x * 2.0f * M_PI); },
    .eval_fixed = [](float x) -> float { return BrFixedToFloat(BrFixedCos(BrFloatToFixed(x))); },
};

const Graph graph_asin = {
    .from       = -1.0f,
    .to         = 1.0f,
    .step       = 0.001f,
    .eval_float = [](float x) -> float { return asin(x); },
    .eval_fixed = [](float x) -> float {
        float v = BrFixedLUFToFloat(BrFixedASin(BrFloatToFixed(x)));

        if(v >= 0.5f)
            v -= 1.0f;

        return BrAngleToRadian(v);
    },
};

const Graph graph_acos = {
    .from       = -1.0f,
    .to         = 1.0f,
    .step       = 0.001f,
    .eval_float = [](float x) -> float { return acos(x); },
    .eval_fixed = [](float x) -> float { return BrAngleToRadian(BrFixedLUFToFloat(BrFixedACos(BrFloatToFixed(x)))); },
};

const Graph graph_rcp = {
    .from       = 0.5f,
    .to         = 1000.0f,
    .step       = 0.5f,
    .eval_float = [](float x) -> float { return 1.0f / x; },
    .eval_fixed = [](float x) -> float { return BrFixedToFloat(BrFixedRcp(BrFloatToFixed(x))); },
};

typedef struct Graph3D {
    float x_from, x_to, x_step;
    float y_from, y_to, y_step;
    float (*eval_float)(float x, float y);
    float (*eval_fixed)(float x, float y);
} Graph3D;

const Graph3D graph3d_atan2 = {
    .x_from     = -65535.0f,
    .x_to       = 65535.0f,
    .x_step     = 1024.0f,
    .y_from     = -65535.0f,
    .y_to       = 65535.0f,
    .y_step     = 1024.0f,
    .eval_float = [](float x, float y) -> float { return atan2f(y, x); },
    .eval_fixed = [](float x, float y) -> float {
        float v = BrFixedLUFToFloat(BrFixedATan2(BrFloatToFixed(y), BrFloatToFixed(x)));

        if(v > 0.5f)
            v -= 1.0f;

        return BrAngleToRadian(v);
    },
};

template <typename F>
void plot2d(const Graph *g, FILE *fp, size_t x_n, F&& fn)
{
    for(size_t x = 0; x < x_n; ++x) {
        float x_f = g->from + (static_cast<float>(x) * g->step);
        fprintf(fp, "%f %f\n", x_f, fn(g, x_f));
    }

    fputs("e\n", fp);
}

template <typename F>
void plot3d(const Graph3D *g, FILE *fp, size_t x_n, size_t y_n, F&& fn)
{
    for(size_t x = 0; x < x_n; ++x) {
        for(size_t y = 0; y < y_n; ++y) {
            float x_f = g->x_from + (static_cast<float>(x) * g->x_step);
            float y_f = g->y_from + (static_cast<float>(y) * g->y_step);
            fprintf(fp, "%f %f %f\n", x_f, y_f, fn(g, x_f, y_f));
        }
    }

    fputs("e\n", fp);
}

static int gnuplot3d(const Graph3D *g, FILE *fp)
{
    setvbuf(fp, buf, _IOFBF, sizeof(buf));
    fprintf(fp, "set terminal qt enhanced\n");

    fprintf(fp, "set xlabel 'x'\n");
    fprintf(fp, "set ylabel 'y'\n");
    fprintf(fp, "set zlabel 'f(x,y)'\n");
    fprintf(fp, "splot '-' using 1:2:3 title 'float', \\\n"
                "      '-' using 1:2:3 title 'fixed', \\\n"
                "      '-' using 1:2:3 title 'diff'\n");

    size_t x_n = static_cast<size_t>((g->x_to - g->x_from) / g->x_step);
    size_t y_n = static_cast<size_t>((g->y_to - g->y_from) / g->y_step);

    plot3d(g, fp, x_n, y_n, [](const Graph3D *g, float x, float y) { return g->eval_float(x, y); });
    plot3d(g, fp, x_n, y_n, [](const Graph3D *g, float x, float y) { return g->eval_fixed(x, y); });
    plot3d(g, fp, x_n, y_n,
           [](const Graph3D *g, float x, float y) { return fabs(fabs(g->eval_float(x, y)) - fabs(g->eval_fixed(x, y))); });

    return 0;
}

static int gnuplot2d(const Graph *g, FILE *fp)
{
    setvbuf(fp, buf, _IOFBF, sizeof(buf));
    fprintf(fp, "set terminal qt enhanced\n");
    fprintf(fp, "set xlabel 'x'\n");
    fprintf(fp, "set ylabel 'f(x)'\n");
    fprintf(fp, "plot '-' using 1:2 with lines title 'float', \\\n"
                "     '-' using 1:2 with lines title 'fixed', \\\n"
                "     '-' using 1:2 with lines title 'diff'\n");

    size_t n = static_cast<size_t>((g->to - g->from) / g->step);

    plot2d(g, fp, n, [](const Graph *g, float x) { return g->eval_float(x); });
    plot2d(g, fp, n, [](const Graph *g, float x) { return g->eval_fixed(x); });
    plot2d(g, fp, n, [](const Graph *g, float x) { return fabs(fabs(g->eval_float(x)) - fabs(g->eval_fixed(x))); });

    return 0;
}

int main(int argc, char **argv)
{
    fprintf(stderr, "Pipe to file and 'load \"file\"' if you want interactive\n");
    //gnuplot2d(&graph_cos, stdout);
    gnuplot3d(&graph3d_atan2, stdout);
    return 0;
}

#if 0
static int gnuplot(size_t n, float *x, float *y_f, float *y_x)
{
    FILE *fp;

    if((fp = popen("gnuplot -persist", "w")) == nullptr)
        return -1;

    setvbuf(fp, buf, _IOFBF, sizeof(buf));
    // fprintf(fp, "set terminal png\n");
    fprintf(fp, "set terminal qt enhanced\n");
    // fprintf(fp, "set output 'plot.png'\n");
    fprintf(fp, "set xlabel 'x'\n");
    fprintf(fp, "set ylabel 'f(x)'\n");
    fprintf(fp, "plot '-' using 1:2 with lines title 'float', \\\n"
                "     '-' using 1:2 with lines title 'fixed', \\\n"
                "     '-' using 1:2 with lines title 'diff'\n");

    /* Write the float data. */
    for(size_t i = 0; i < n; ++i) {
        fprintf(fp, "%f %f\n", x[i], y_f[i]);
    }
    fputs("e\n", fp);

    /* Write the fixed data. */
    for(size_t i = 0; i < n; ++i) {
        fprintf(fp, "%f %f\n", x[i], y_x[i]);
    }
    fputs("e\n", fp);

    /* Write the diff data. */
    for(size_t i = 0; i < n; ++i) {
        fprintf(fp, "%f %f\n", x[i], fabs(fabs(y_f[i]) - fabs(y_x[i])));
    }
    fputs("e\n", fp);

    pclose(fp);
    return 0;
}

int main2d(int argc, char **argv)
{
    const float from = graph.from;
    const float to   = graph.to;
    const float step = graph.step;

    size_t n = static_cast<size_t>((to - from) / step);

    std::unique_ptr<float[]> data_x   = std::make_unique<float[]>(n);
    std::unique_ptr<float[]> data_y_f = std::make_unique<float[]>(n);
    std::unique_ptr<float[]> data_y_x = std::make_unique<float[]>(n);

    data_x[0] = graph.from;
    for(size_t i = 1; i < n; ++i) {
        data_x[i] = data_x[i - 1] + step;
    }

    for(size_t i = 0; i < n; ++i) {
        float x     = data_x[i];
        data_y_f[i] = graph.eval_float(x);
    }

    for(size_t i = 0; i < n; ++i) {
        float x     = data_x[i];
        data_y_x[i] = graph.eval_fixed(x);
    }

    gnuplot(n, data_x.get(), data_y_f.get(), data_y_x.get());
    return 0;
}
#endif