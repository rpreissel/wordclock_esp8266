import { ColorMap } from "./types";
import { rgbToHex } from "./colors";

type ColorChooserProps = {
  value: string;
  showOffColor?: boolean;
  onChange: (color: string) => void;
  colors: ColorMap;
}

export const ColorChooser = ({ value, onChange, showOffColor=false, colors }: ColorChooserProps) => {
  return <div className="d-flex">
    <div style={{ width: "fit-content", margin: "auto" }}>

      {Object.keys(colors).map((colorIndex, i) => {
        const go = colors[colorIndex];
        const active = value === go.name;
        const classes = "color btn m-2 overflow-hidden " + (active ? "active-color" : "");

        return i || showOffColor ?
          <button key={i} disabled={active} style={{ backgroundColor: rgbToHex(go) }} className={classes} onClick={() => onChange(go.name)} />
          : <div key={i}></div>

      })}
    </div>
  </div>;
}

export default ColorChooser;