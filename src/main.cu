#include <iostream>
#include <string>
#include <thread>

#include "flann/flann.h"
#include "flann/flann.hpp"

#include "tools/project.h"
#include "tools/io.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
	#ifndef DEBUG_NEW
		#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
		#define new DEBUG_NEW
	#endif
#endif

int main(int argc, char* argv[]) {
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	std::cout << "----------------------- Main -----------------------" << std::endl;;

	int i = 0;
	int cores = (unsigned int)std::thread::hardware_concurrency();
	while (i < argc) {
		if (!strcmp(argv[i], "--cores")) {
			i++;
			cores = std::stoi(argv[i]);
		}
		i++;
	}

	char *file = "C:/Users/Wolfgang Brandenburg/OneDrive/Dokumente/3DModelle/Sonstiges/plane.ply";

	utils::Timer time;
	utils::Pointcloud<float> pointcloud;

	time.start();
	if (io::readply<float>(file, pointcloud)) {
		std::cout << "File with " << pointcloud.points.rows << " point has been read in "
			<< time.stop() << " s into Pointcloud" << std::endl;
	}

	flann::Matrix<float> pointcloudflann(pointcloud.getPointsPtr(), pointcloud.rows, pointcloud.cols);



	////////// allocate indices- and dists-matrices 
	////////int nn = 20;
	////////int querynumber = pointcloudflann.rows;
	////////
	////////flann::Matrix<size_t> indices(new size_t[querynumber*nn], querynumber, nn);
	////////flann::Matrix<float> dists(new float[querynumber*nn], querynumber, nn);
	////////for (int i = 0; i < querynumber; i++) {
	////////	for (int j = 0; j < nn; j++) {
	////////		indices[i][j] = 0;
	////////		dists[i][j] = 0;
	////////	}
	////////}

	////////// build index and perform knn-search
	////////time.start();
	////////flann::Index<flann::L2<float>> index(pointcloudflann, flann::KDTreeCudaIndexParams(5));
	////////index.buildIndex();

	////////std::cout << "kd-tree has been built in " << time.stop() << " s" << std::endl;

	////////flann::SearchParams params;
	////////params.checks = 32;
	////////params.cores = cores;

	////////time.start();
	////////index.knnSearch(pointcloudflann, indices, dists, nn, params);
	////////std::cout << "search has been performed in " << time.stop() << " s" << std::endl;




	// allocate indices- and dists-matrices 
	int nn = 1000;
	int querynumber = 32;// pointcloudflann.rows;
	
	flann::Matrix<size_t> indices(new size_t[querynumber*nn], querynumber, nn);
	flann::Matrix<float> dists(new float[querynumber*nn], querynumber, nn);
	for (int i = 0; i < querynumber; i++) {
		for (int j = 0; j < nn; j++) {
			indices[i][j] = 0;
			dists[i][j] = 0;
		}
	}

	utils::rand_seed();
	flann::Matrix<float> query(new float[querynumber*pointcloudflann.cols], querynumber, pointcloudflann.cols);
	for (int i = 0; i < querynumber; i++) {
		int random = utils::rand<int>(pointcloudflann.rows - 1, 0);
		for (int j = 0; j < pointcloudflann.cols; j++) {
			query[i][j] = pointcloudflann[random][j];
		}
	}

	// build index and perform knn-search
	time.start();
	flann::Index<flann::L2<float>> index(pointcloudflann, flann::KDTreeCudaIndexParams(5));
	index.buildIndex();

	std::cout << "kd-tree has been built in " << time.stop() << " s" << std::endl;

	flann::SearchParams params;
	params.checks = 32;
	params.cores = cores;

	time.start();
	index.knnSearch(query, indices, dists, nn, params);
	std::cout << "search has been performed in " << time.stop() << " s" << std::endl;

	utils::rand_seed();
	for (int i = 0; i < indices.rows; i++) {
		int r = utils::rand<int>(255, 0);
		int g = utils::rand<int>(255, 0);
		int b = utils::rand<int>(255, 0);
		for (int j = 0; j < indices.cols; j++) {
			pointcloud.colors[indices[i][j]][0] = r;
			pointcloud.colors[indices[i][j]][1] = g;
			pointcloud.colors[indices[i][j]][2] = b;
		}
	}

	io::writeply("C:/Users/Wolfgang Brandenburg/OneDrive/Dokumente/3DModelle/Sonstiges/planeOut.ply", pointcloud);

	// destroy the flann::matrix
	pointcloud.clear();
	delete[] indices.ptr();
	delete[] dists.ptr();

	return(0);
}