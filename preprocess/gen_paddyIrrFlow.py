#! /usr/bin/env python
# coding=utf-8
#
# Author: Shen fang

import os
import math
import numpy
import sqlite3
from util import *
from config import *
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

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
                    # print cell_index,len(nCells) - 1,data_landuse[i][j],pond_id,reach_id

                    cell_id = len(nCells) - 1
                    flow_table.append([cell_id, pond_id,reach_id])
    f = open(txtName,'a')
    for i in range(len(flow_table)):
        for j in range(3):
            f.write(str(flow_table[i][j]) + '\t')
        f.write("\n")
    f.close

def ImportPaddyPondFlow(db):
    # delete if existed, create if not existed
    cList = db.collection_names()
    if not StringInList(DB_TAB_PADDYPONDFLOW.upper(), cList):
        db.create_collection(DB_TAB_PADDYPONDFLOW.upper())
    else:
        db.drop_collection(DB_TAB_PADDYPONDFLOW.upper())
    # print cList
    dataItems = ReadDataItemsFromTxt(txtName)
    for id in range(len(dataItems)):
        dic = {}
        dic[PADDYPONDFLOW_PADDYID.upper()] = dataItems[id][0]
        dic[PADDYPONDFLOW_PONDID.upper()] = dataItems[id][1]
        dic[PADDYPONDFLOW_REACHID.upper()] = dataItems[id][2]
        db[DB_TAB_PADDYPONDFLOW.upper()].find_one_and_replace(dic, dic, dic, upsert=True)

    print 'Paddy pond flow tables are imported.'

if __name__ == '__main__':
    path = r"J:\seims_paddy\paddyIrr"
    dem  = path +  os.sep + "dem.tif"
    landuse = path +  os.sep + "landuse.tif"
    subbasin = path +  os.sep + "SUBBASIN.tif"
    pond = path +  os.sep + "pond.tif"
    txtName = path + os.sep + "paddy_pond_flow.txt"
    # find_neighbour_pond(dem, landuse, subbasin, pond)

    # Load Configuration file
    LoadConfiguration(GetINIfile())
    import sys
    try:
        conn = MongoClient(HOSTNAME, PORT)
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    db = conn[SpatialDBName]

    ImportPaddyPondFlow(db)