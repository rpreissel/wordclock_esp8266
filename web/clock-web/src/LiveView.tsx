import { Button, Form } from "react-bootstrap";
import { ColorMap, Configs, FixedTime, LiveViewData, Mode, modeName } from "./types";
import { useEffect, useState } from "react"

import { colorIndexToHex } from "./colors";
import { range } from "./range";

type LiveRowProps = {
  colors: ColorMap;
  textString: string;
  colorString: string;
};

const LiveRow = ({ textString, colorString, colors }: LiveRowProps) => {
  return <>
    {range(0, 11).map((col) => {
      const color = col < colorString.length && colorString[col] != ' ' ? colorIndexToHex(+colorString[col], colors) : undefined;
      return <div key={col} className="livecell" style={{ color: color }}>
        {color && col < textString.length ? textString[col] : ' '}
      </div>;
    })
    }
  </>
}

type LiveViewProps = {
  modes: Mode[];
  fixedTime: FixedTime;
  configs: Configs;
  onChange: (fixedTime: FixedTime) => void;
  onResetWifi: () => void;
  onResetData: () => void;
};
const LiveView = ({ modes, configs: { colors, leds }, fixedTime, onChange, onResetWifi, onResetData }: LiveViewProps) => {

  const [data, setData] = useState<LiveViewData | undefined>()
  const [wlanReset, setWlanReset] = useState<string>("")
  const [dataReset, setDataReset] = useState<string>("")

  useEffect(() => {
    function loadLiveView() {
      fetch('./api/live')
        .then(result => result.json())
        .then(result => setData(result))
    }
    loadLiveView()
    const interval = setInterval(() => loadLiveView(), 1000)
    return () => {
      clearInterval(interval);
    }
  }, [])

  if (!data) {
    return <></>;
  }
  const hours = fixedTime.enabled ? fixedTime.hours : data.time.hours;
  const minutes = fixedTime.enabled ? fixedTime.minutes : data.time.minutes;
  return (
    <>
      <div>
        <Form >
          <Form.Group controlId="formModes" className="mb-1">
            <Form.Label>
              Aktiver Mode
            </Form.Label>
            <Form.Control type="text" readOnly value={data.activemodes.map(index => modeName(modes[index])).join(" / ")} />
          </Form.Group>
          <Form.Group controlId="formTimer" className="mb-3">
            <Form.Label>
              Aktuelle Zeit
            </Form.Label>
            <div className="d-flex flex-wrap">
              <div className="me-3 flex-fill">
                <Form.Control type="time"
                  size="sm"
                  value={hours.toString().padStart(2, '0') + ":" + minutes.toString().padStart(2, '0')}
                  readOnly={!fixedTime.enabled}
                  onChange={e => {
                    const [newHour, newMinute] = e.currentTarget.value.split(":");
                    onChange({ enabled: true, hours: +newHour, minutes: +newMinute })

                  }} />
              </div>
              <div className="p-1">
                <Form.Check // prettier-ignore
                  type="switch"
                  label="Fix Time"
                  checked={fixedTime.enabled}
                  onChange={() => {
                    if (fixedTime.enabled) {
                      onChange({ enabled: false, hours: fixedTime.hours, minutes: fixedTime.minutes })
                    } else {
                      onChange({ enabled: true, hours: hours, minutes: minutes })
                    }
                  }}
                />
              </div>
            </div>
          </Form.Group>
          <Form.Group className="livegrid mb-3">
            {range(0, 11).map((row) =>
              <LiveRow key={row} textString={leds[row.toString(16)]} colorString={data.colors[row.toString(16)]} colors={colors} />
            )}
            <LiveRow key="M" textString={leds.M} colorString={"   " + data.colors.M} colors={colors} />
          </Form.Group>
          <Form.Group controlId="formWlanReset" className="mb-1">
            <Form.Label>Wlan Zurücksetzen</Form.Label>
            <div className="d-flex">
              <div className="d-inline-block p-1">
                <Form.Control type="text"
                  size="sm"
                  minLength={300}
                  value={wlanReset}
                  placeholder="Tippe 'Ja'"
                  onChange={e =>
                    setWlanReset(e.currentTarget.value)
                  } />
              </div>
              <div className="d-inline-block w-25 p-1">
                <Button className="action" type="button" size="sm" variant="danger" disabled={wlanReset != 'Ja'} onClick={() => {
                  onResetWifi();
                  setWlanReset("");
                }}>
                  Reset
                </Button>
              </div>

            </div>
          </Form.Group>
          <Form.Group controlId="formDataReset">
            <Form.Label>Daten Zurücksetzen</Form.Label>
            <div className="d-flex">
              <div className="d-inline-block p-1">
                <Form.Control type="text"
                  size="sm"
                  minLength={300}
                  value={dataReset}
                  placeholder="Tippe 'Ja'"
                  onChange={e =>
                    setDataReset(e.currentTarget.value)
                  } />
              </div>
              <div className="d-inline-block w-25 p-1">
                <Button className="action" type="button" size="sm" variant="danger" disabled={dataReset != 'Ja'} onClick={() => {
                  onResetData();
                  setDataReset("");
                }}>
                  Reset
                </Button>
              </div>

            </div>
          </Form.Group>
        </Form >
      </div >

    </>
  )
}

export default LiveView;