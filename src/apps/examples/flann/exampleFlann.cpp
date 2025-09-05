/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file exampleFlann.cpp @brief FLANN example. */

// Include 3rd party.
#include <flann/flann.hpp>

// Include 3D Forest.
#include <Error.hpp>

// Include local.
#define LOG_MODULE_NAME "exampleFlann"
#include <Log.hpp>

using namespace flann;

static void exampleFlann()
{
    int nn = 3;

    // data
    float d[] = {0.0F, 0.0F, 0.0F};
    float q[] = {0.0F, 0.0F, 0.0F};

    Matrix<float> dataset(d, 1, 3);
    Matrix<float> query(q, 1, 3);

    Matrix<int> indices(new int[query.rows * nn], query.rows, nn);
    Matrix<float> dists(new float[query.rows * nn], query.rows, nn);

    // construct an randomized kd-tree index using 4 kd-trees
    Index<L2<float>> index(dataset, flann::KDTreeIndexParams(4));
    index.buildIndex();

    // do a knn search, using 128 checks
    index.knnSearch(query, indices, dists, nn, flann::SearchParams(128));

    // print
    for (size_t i = 0; i < indices.rows * indices.cols; i++)
    {
        std::cout << *indices[i] << std::endl;
    }
    std::cout << std::endl;

    delete[] indices.ptr();
    delete[] dists.ptr();
}

int main()
{
    try
    {
        exampleFlann();
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
