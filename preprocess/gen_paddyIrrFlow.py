#! /usr/bin/env python
# coding=utf-8
#
# Author: Shen fang

import os
import math
import numpy
import sqlite3
import arcpy
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

def ReadRaster(raster):
    raster = gdal.Open(raster)
    band_raster = raster.GetRasterBand(1)
    data_raster = band_raster.ReadAsArray()
    return data_raster

def find_neighbour_pond(dem, landuse, subbasin, pond):
    # read col, row, cell width from dem
    dem = gdal.Open(dem)
    band_dem = dem.GetRasterBand(1)
    data_dem = band_dem.ReadAsArray()
    nCols = band_dem.XSize
    nRows = band_dem.YSize
    geotrans = dem.GetGeoTransform()
    cell_width = geotrans[1]

    # read data value of landuse, subbasin, pond
    data_landuse = ReadRaster(landuse)
    data_subbasin = ReadRaster(subbasin)
    data_pond = ReadRaster(pond)

    # set some parameters to search nearest pond
    maxLength = 1000.
    maxNum = 3
    num = int(maxLength / cell_width / 2.)
    nCells = []

    for i in range(nRows):
        for j in range(nCols):
            if data_landuse[i][j] != DEFAULT_NODATA:
                cell_index = i * nCols + j
                nCells.append(cell_index)
                if data_landuse[i][j] == 33.:
                    # get paddy cell id, which is corresponding to the id in the main module
                    cell_id = len(nCells) - 1
                    reach_id = data_subbasin[i][j]
                    dic = {}
                    for k in range(max(0, i - num), min(i + num, nRows)):
                        for m in range(max(0, j - num), min(j + num, nCols)):
                            # for a paddy cell id, search the window (1km * 1km),
                            # if the cell is pond and in the subbasin, then compute the distance
                            if data_subbasin[k][m] == reach_id:
                                if data_landuse[k][m] == 200.:
                                    # the pond id
                                    index = data_pond[k][m]
                                    distance = cell_width * math.sqrt(pow(k - i, 2) + pow(m - j, 2))
                                    dic[index] = distance
                    # sort the dic by distance , and choose the 5 nearest pond, if not enough ,then make it -9999
                    dic_sort = sorted(dic.iteritems(), key=lambda t:t[1], reverse=False)
                    if len(dic_sort) >= maxNum:
                        for n in range(maxNum):
                            locals()['nearest_pond_id_%s' % n]  = dic_sort[n][0]
                    else:
                        for n in range(len(dic_sort)):
                            locals()['nearest_pond_id_%s' % n] = dic_sort[n][0]
                        for n in range(len(dic_sort), maxNum):
                            locals()['nearest_pond_id_%s' % n] = DEFAULT_NODATA

                    # write to txt
                    flow_table = [cell_id, reach_id]
                    for n in range(maxNum):
                        flow_table.append(locals()['nearest_pond_id_%s' % n])
                    f = open(txtName, 'a')
                    for s in range(len(flow_table)):
                        f.write(str(flow_table[s]) + '\t')
                    f.write("\n")
                    f.close

def ImportPaddyPondFlow(db):
    # delete if existed, create if not existed
    cList = db.collection_names()
    if not StringInList(DB_TAB_POND.upper(), cList):
        db.create_collection(DB_TAB_POND.upper())
    else:
        db.drop_collection(DB_TAB_POND.upper())

    dataItems = ReadDataItemsFromTxt(txtName)
    for id in range(len(dataItems)):
        dic = {}
        dic[POND_PADDYID.upper()] = dataItems[id][0]
        dic[POND_REACHID.upper()] = dataItems[id][1]
        dic[POND_PONDID1.upper()] = dataItems[id][2]
        dic[POND_PONDID2.upper()] = dataItems[id][3]
        dic[POND_PONDID3.upper()] = dataItems[id][4]
        db[DB_TAB_POND.upper()].insert(dic)

    print 'Paddy pond flow tables are imported.'

if __name__ == '__main__':
    path = r"J:\seims_paddy\paddyIrr"
    dem  = path +  os.sep + "dem.tif"
    landuse = path +  os.sep + "landuse.tif"
    subbasin = path +  os.sep + "SUBBASIN.tif"
    pond = path +  os.sep + "pond.tif"
    txtName = path + os.sep + "paddy_pond_flow.txt"
    pond_shp = path + os.sep + "pond.shp"
    qjrws = arcpy.SearchCursor(pond_shp)
    pond_area = {}
    for qjrw in qjrws:
        id = qjrw.getValue("PONDID")
        pond_area[id] = qjrw.getValue("Shape_Area")
    # find_neighbour_pond(dem, landuse, subbasin, pond)

    #Load Configuration file
    LoadConfiguration(GetINIfile())
    import sys
    try:
        conn = MongoClient(HOSTNAME, PORT)
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    db = conn[SpatialDBName]
    # ImportPaddyPondFlow(db)

    # import pond raster to mongodb
    tifFolder = WORKING_DIR + os.sep + DIR_NAME_TIFFIMPORT
    mask = r"J:\seims_paddy\zts_output\mask.tif"
    pond_dir = "J:\seims_paddy\pond"

    strCmd = '"%s/import_raster" %s %s %s %s %s %d %s' % (
        CPP_PROGRAM_DIR, mask, pond_dir, SpatialDBName,
        DB_TAB_SPATIAL.upper(), HOSTNAME, PORT, tifFolder)
    print strCmd
    RunExternalCmd(strCmd)