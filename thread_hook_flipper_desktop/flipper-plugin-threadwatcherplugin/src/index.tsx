/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {DataTableColumn, createTablePlugin} from 'flipper-plugin';

type Row = {
  id: number;
  tid: number;
  name: string;
  threadGroup:string;
  priority:string;
  createCallStack: string;
};

const columns: DataTableColumn<Row>[] = [
  {
    key: 'tid',
    width: 150,
  },
  {
    key: 'name',
    width: 150,
  },
  {
  key:'threadGroup',
  width:150,
  },
    {
    key:'priority',
    width:150,
    },
  {
    key: 'createCallStack',
    title: 'createCallStack',
  },
];

/**
 * Table based plugin, based on the table tutorial:
 * https://fbflipper.com/docs/tutorial/js-table
 *
 * For a custom Plugin layout, update this file according to the custom plugin tutorial:
 * https://fbflipper.com/docs/tutorial/js-custom
 *
 * The full API of createTablePlugin can be found here:
 * https://fbflipper.com/docs/extending/flipper-plugin#createtableplugin
 */
module.exports = createTablePlugin<Row>({
  columns,
  key: 'tid',
  method: 'newThread',
});