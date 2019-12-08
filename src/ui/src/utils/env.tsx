import * as moment from 'moment';

const {
  BUILD_ENV,
  BUILD_NUMBER,
  BUILD_SCM_REVISION,
  BUILD_SCM_STATUS,
  BUILD_TIMESTAMP,
} = process.env;

const timestampSec = Number.parseInt(BUILD_TIMESTAMP, 10);
const date = isNaN(timestampSec) ? new Date() : new Date(timestampSec * 1000);
const dateStr = moment(date).utc().format('YYYY.MM.DD.hh.mm');
const parts = [];
if (typeof BUILD_SCM_REVISION === 'string') {
  parts.push(BUILD_SCM_REVISION.substr(0, 7));
}
if (!!BUILD_SCM_STATUS) {
  parts.push(BUILD_SCM_STATUS);
}
parts.push(isNaN(timestampSec) ? Math.floor(date.valueOf() / 1000) : timestampSec);
if (!!BUILD_NUMBER) {
  parts.push(BUILD_NUMBER);
}

export const PIXIE_CLOUD_VERSION = `${dateStr}+${parts.join('.')}`;

export function isProd(): boolean {
  return (BUILD_ENV || '').toLowerCase() === 'prod';
}
