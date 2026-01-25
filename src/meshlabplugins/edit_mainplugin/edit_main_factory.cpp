/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005-2008                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include "edit_main_factory.h"
#include "edit_pickandalign.h"
#include "edit_slice.h"

MainEditFactory::MainEditFactory()
{
	editPickAndAlign = new QAction(QIcon(":/images/icon_info.png"),tr("Pick And Align"), this);
	editSlice = new QAction(QIcon(":/images/slice.png"), tr("Slice Model"), this);
	
	actionList.push_back(editPickAndAlign);
	actionList.push_back(editSlice);
	
	foreach(QAction *editAction, actionList)
		editAction->setCheckable(true); 	
}

QString MainEditFactory::pluginName() const
{
	return "EditMain";
}

//get the edit tool for the given action
EditTool* MainEditFactory::getEditTool(const QAction *action)
{
	if(action == editPickAndAlign)
	{
		return new PickAndAlignEditPlugin();
	}
	else if  (action == editSlice) {
		return new SliceEditPlugin();
	}
	else assert(0); //should never be asked for an action that isn't here
	return nullptr;
}

QString MainEditFactory::getEditToolDescription(const QAction *action)
{
	if (action == editPickAndAlign)
	{
		return PickAndAlignEditPlugin::info();
	}
	else if (action == editSlice) {
		return SliceEditPlugin::info();
	}	
}

MESHLAB_PLUGIN_NAME_EXPORTER(MainEditFactory)
