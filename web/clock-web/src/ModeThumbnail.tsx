import { ColorMap, Mode } from "./types";

import { colorNameToHex } from "./Colors";

type ModeThumbnailProps = {
  mode: Mode;
  colors: ColorMap;
  active: boolean;
  onClick: (index:number) => void;
};


function ModeThumbnail({ mode, colors, active, onClick }: ModeThumbnailProps) {
  if (mode.type == "EMPTY") {
    return <></>;
  }
  const classes = "thumbnail btn m-2 overflow-hidden " + (active ? "active-thumbnail" : "");
  if (mode.type == "OFF") {
    return (
        <button style={{ backgroundColor: "lightgray"}} className={classes} onClick={() =>onClick(mode.index)}>Off</button>
    );
  }

  return (
        <button style={{ backgroundColor: colorNameToHex(mode.color, colors)}} className={classes} onClick={() =>onClick(mode.index)}>
          {mode.name}
        </button>
  );
}

export default ModeThumbnail;
