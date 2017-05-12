#ifndef IOPLY_HPP_
#define IOPLY_HPP_

#include "ioply.h"

static int counter;
static int elements = 2;

typedef unsigned char uchar_t;

template <typename ElementType> static int callbackMatrix(p_ply_argument argument_)
{
	long index;
	utils::Matrix<ElementType> *dataset;
	ply_get_argument_user_data(argument_, (void**)&dataset, &index);

	switch (index) {
	case 0:
		(*dataset)[counter][0] = (ElementType)ply_get_argument_value(argument_);//	
		break;
	case 1:
		(*dataset)[counter][1] = (ElementType)ply_get_argument_value(argument_);//
		break;
	case 2:		
		(*dataset)[counter][2] = (ElementType)ply_get_argument_value(argument_);//
		counter++;
		break;
	default:
		break;
	}

	return 1;
}

template <typename ElementType> int io::readply(char *stringfile_, utils::Matrix<ElementType>& dataset_)
{
	counter = 0;

	p_ply ply = ply_open(stringfile_, NULL, 0, NULL);
	if (!ply) {
		std::cout << stringfile_ << " not found" << std::endl;
		return 0;
	}

	if (!ply_read_header(ply)) {
		return 0;
	}

	int numberofpoints = ply_set_read_cb(ply, "vertex", "x", callbackMatrix<ElementType>, &dataset_, 0);
	ply_set_read_cb(ply, "vertex", "y", callbackMatrix<ElementType>, &dataset_, 1);
	ply_set_read_cb(ply, "vertex", "z", callbackMatrix<ElementType>, &dataset_, 2);

	dataset_ = utils::Matrix<ElementType>(new ElementType[numberofpoints * 3], numberofpoints, 3);

	if (!ply_read(ply)) {
		return 0;
	}

	ply_close(ply);

	return 1;
}

template <typename ElementType> int io::writeply(char *stringfile_, utils::Matrix<ElementType>& dataset_)
{
	p_ply ply = ply_create(stringfile_, PLY_DEFAULT, NULL, 0, NULL);

	ply_add_element(ply, "vertex", (long)dataset_.rows);

	if (std::is_same<ElementType, float>::value) {
		ply_add_property(ply, "x", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32);
		ply_add_property(ply, "y", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32);
		ply_add_property(ply, "z", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32);
	}
	else {
		ply_add_property(ply, "x", PLY_DOUBLE, PLY_DOUBLE, PLY_DOUBLE);
		ply_add_property(ply, "y", PLY_DOUBLE, PLY_DOUBLE, PLY_DOUBLE);
		ply_add_property(ply, "z", PLY_DOUBLE, PLY_DOUBLE, PLY_DOUBLE);
	}

	ply_write_header(ply);

	for (int i = 0; i < dataset_.rows; i++) {
		ply_write(ply, dataset_[i][0]);
		ply_write(ply, dataset_[i][1]);
		ply_write(ply, dataset_[i][2]);
	}

	ply_close(ply);

	return 1;
}

template <typename ElementType> static int callbackPointcloud(p_ply_argument argument)
{
	long index;
	utils::Pointcloud<ElementType> *pointcloud;
	ply_get_argument_user_data(argument, (void**)&pointcloud, &index);

	switch (index) {
	case 0:
		(*pointcloud).points[counter][0] = (ElementType)ply_get_argument_value(argument);
		break;
	case 1:
		(*pointcloud).points[counter][1] = (ElementType)ply_get_argument_value(argument);
		break;
	case 2:
		(*pointcloud).points[counter][2] = (ElementType)ply_get_argument_value(argument);
		if (elements == 2) {
			counter++;
		}
		break;
	case 3:
		(*pointcloud).colors[counter][0] = (uchar_t)ply_get_argument_value(argument);
		break;
	case 4:
		(*pointcloud).colors[counter][1] = (uchar_t)ply_get_argument_value(argument);
		break;
	case 5:
		(*pointcloud).colors[counter][2] = (uchar_t) ply_get_argument_value(argument);
		if (elements == 5) {
			counter++;
		}
		break;
	default:
		break;
	}

	return 1;
}

template <typename ElementType> int io::readply(char *stringfile_, utils::Pointcloud<ElementType>& pointcloud_)
{
	counter = 0;

	p_ply ply = ply_open(stringfile_, NULL, 0, NULL);
	if (!ply) {
		std::cout << stringfile_ << " not found" << std::endl;
		return 0;
	}

	if (!ply_read_header(ply)) {
		return 0;
	}

	int numberofpoints = ply_set_read_cb(ply, "vertex", "x", callbackPointcloud<ElementType>, &pointcloud_, 0);
	ply_set_read_cb(ply, "vertex", "y", callbackPointcloud<ElementType>, &pointcloud_, 1);
	ply_set_read_cb(ply, "vertex", "z", callbackPointcloud<ElementType>, &pointcloud_, 2);

	if (numberofpoints) {
		pointcloud_.setPoints(numberofpoints, 3);
	}
	else {
		return 0;
	}

	int numberofcolors = ply_set_read_cb(ply, "vertex", "diffuse_red", callbackPointcloud<ElementType>, &pointcloud_, 3);
	ply_set_read_cb(ply, "vertex", "diffuse_green", callbackPointcloud<ElementType>, &pointcloud_, 4);
	ply_set_read_cb(ply, "vertex", "diffuse_blue", callbackPointcloud<ElementType>, &pointcloud_, 5);

	if (!numberofcolors) {
		numberofcolors = ply_set_read_cb(ply, "vertex", "red", callbackPointcloud<ElementType>, &pointcloud_, 3);
		ply_set_read_cb(ply, "vertex", "green", callbackPointcloud<ElementType>, &pointcloud_, 4);
		ply_set_read_cb(ply, "vertex", "blue", callbackPointcloud<ElementType>, &pointcloud_, 5);
	}

	if (numberofcolors) {
		pointcloud_.setColors(numberofcolors, 3);
		elements = 5;
	}

	if (!ply_read(ply)) {
		return 0;
	}

	ply_close(ply);

	return 1;
}

template <typename ElementType> int io::writeply(char *stringfile_, utils::Pointcloud<ElementType>& pointcloud_)
{
	p_ply ply = ply_create(stringfile_, PLY_DEFAULT, NULL, 0, NULL);

	ply_add_element(ply, "vertex", (long)pointcloud_.points.rows);

	if (std::is_same<ElementType, float>::value) {
		ply_add_property(ply, "x", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32);
		ply_add_property(ply, "y", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32);
		ply_add_property(ply, "z", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32);
	}
	else {
		ply_add_property(ply, "x", PLY_DOUBLE, PLY_DOUBLE, PLY_DOUBLE);
		ply_add_property(ply, "y", PLY_DOUBLE, PLY_DOUBLE, PLY_DOUBLE);
		ply_add_property(ply, "z", PLY_DOUBLE, PLY_DOUBLE, PLY_DOUBLE);
	}

	if (pointcloud_.colors.rows > 0) {
		ply_add_property(ply, "red", PLY_UCHAR, PLY_UCHAR, PLY_UCHAR);
		ply_add_property(ply, "green", PLY_UCHAR, PLY_UCHAR, PLY_UCHAR);
		ply_add_property(ply, "blue", PLY_UCHAR, PLY_UCHAR, PLY_UCHAR);
	}

	ply_write_header(ply);

	for (int i = 0; i < pointcloud_.points.rows; i++) {
		ply_write(ply, pointcloud_.points[i][0]);
		ply_write(ply, pointcloud_.points[i][1]);
		ply_write(ply, pointcloud_.points[i][2]);
		
		if (pointcloud_.colors.rows > 0) {
			ply_write(ply, pointcloud_.colors[i][0]);
			ply_write(ply, pointcloud_.colors[i][1]);
			ply_write(ply, pointcloud_.colors[i][2]);
		}
	}

	ply_close(ply);

	return 1;
}

#endif /* IOPLY_HPP_*/