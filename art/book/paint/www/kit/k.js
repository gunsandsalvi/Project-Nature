// Settings shared by the kit while a scene is built: metres per art pixel, so cards show their tiles one texel
// to one art pixel. The tiles are drawn in metres (atlas.js), so a card of card() metres shows them true to size.
import { T } from '../atlas.js';

export const K = { mpp: 0.06 };

/** The size in metres of a card that shows its tile one texel to one art pixel. */
export const card = () => T * K.mpp;
