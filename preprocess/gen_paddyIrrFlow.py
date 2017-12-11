#! /usr/bin/env python
# coding=utf-8
#
# Author: Shen fang

import os
import math
from util import *


def GetValueByRowCol(row, col, nRows, nCols, data):
    if row < 0 or row >= nRows or col < 0 or col >= nCols:
        raise ValueError(
            "The row or col must be >=0 and less than nRows or nCols!")
    else:
        value = data[int(round(row))][int(round(col))]
        if value == DEFAULT_NODATA:
            return None
        else:
            return value

def find_neighbour_pond(dem, landuse, subbasin, pond):
    dem = gdal.Open(dem)
    band_dem = dem.GetRasterBand(1)
    data_dem = band_dem.ReadAsArray()
    nCols = band_dem.XSize
    nRows = band_dem.YSize
    geotrans = dem.GetGeoTransform()
    cell_width = geotrans[1]
    # print xsize, ysize,cell_width
    landuse = gdal.Open(landuse)
    band_landuse = landuse.GetRasterBand(1)
    data_landuse = band_landuse.ReadAsArray()
    subbasin = gdal.Open(subbasin)
    band_subbasin = subbasin.GetRasterBand(1)
    data_subbasin = band_subbasin.ReadAsArray()

    pond = gdal.Open(pond)
    band_pond = pond.GetRasterBand(1)
    data_pond = band_pond.ReadAsArray()
    # xsize1 = band_landuse.XSize
    # ysize1 = band_landuse.YSize
    maxLength = 1000.
    num = int(maxLength / cell_width / 2.)
    nCells = []
    flow_table = []
    for i in range(nRows):
        for j in range(nCols):
            if data_landuse[i][j] != DEFAULT_NODATA:
                cell_index = i * nCols + j
                nCells.append(cell_index)
                if data_landuse[i][j] == 33.:
                    reach_id = data_subbasin[i][j]
                    dic = {}
                    for k in range(max(0, i - num), min(i + num, nRows)):
                        for m in range(max(0, j - num), min(j + num, nCols)):
                            if data_landuse[k][m] == 200.:
                                index = data_pond[k][m]
                                distance = math.sqrt(cell_width * (pow(k - i, 2) + pow(m - j, 2)) + pow(data_dem[k][m] - data_dem[i][j], 2))
                                dic[index] = distance

                    if dic:
                        pond_id = min(dic, key=lambda x: dic[x])
                    else:
                        pond_id = DEFAULT_NODATA
                    print cell_index,len(nCells) - 1,data_landuse[i][j],pond_id,reach_id


                # print reach_id
    # print len(nCells)

    # print xsize1,ysize1
    # print data[0]

if __name__ == '__main__':
    path = r"J:\seims_paddy\paddyIrr"
    dem  = path +  os.sep + "dem.tif"
    landuse = path +  os.sep + "landuse.tif"
    subbasin = path +  os.sep + "SUBBASIN.tif"
    pond = path +  os.sep + "pond.tif"
    find_neighbour_pond(dem, landuse, subbasin , pond)