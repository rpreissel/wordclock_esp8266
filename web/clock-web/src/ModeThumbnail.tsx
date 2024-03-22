import { Mode } from "./types";

type ModeThumbnailProps = {
  mode: Mode;
  color: string;
  active: boolean;
  onClick: (index:number) => void;
};


function ModeThumbnail({ mode, color, active, onClick }: ModeThumbnailProps) {
  if (mode.type == "EMPTY") {
    return <></>;
  }
  const classes = "thumbnail btn m-2 overflow-hidden " + (active ? "active-thumbnail" : "");
  if (mode.type == "OFF") {
    return (
        <button style={{ backgroundColor: color}} className={classes} onClick={() =>onClick(mode.index)}>Off</button>
    );
  }

  return (
        <button style={{ backgroundColor: color}} className={classes} onClick={() =>onClick(mode.index)}>
          {mode.name}
        </button>
  );
}

export default ModeThumbnail;
