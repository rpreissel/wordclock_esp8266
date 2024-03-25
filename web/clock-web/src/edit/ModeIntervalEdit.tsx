import { BaseMode, Configs, IntervalMode, Mode } from "../types";
import { Button, Form } from "react-bootstrap";

type ModeIntervalEditProps = {
  mode: IntervalMode;
  modes: Mode[];
  configs: Configs;
  onChange: (mode: IntervalMode) => void;
};
export function ModeIntervalEdit({ mode, modes, onChange }: ModeIntervalEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formInterval">
      <div className="d-flex">
        <div className="d-inline-block w-25 p-1">
          <Form.Label>Seconds</Form.Label>
        </div>
        <div className="d-inline-block flex-fill p-1">
          <Form.Label>Mode</Form.Label>
        </div>
      </div>
      {mode.intervals.map((interval, i) => {
        return (<div key={i} className="d-flex">
          <div className="d-inline-block w-25 p-1">
            <Form.Control type="number" value={interval.seconds} onChange={e => {
              const newinterval = { ...interval, seconds: +e.currentTarget.value };
              const newIntervals = [...mode.intervals];
              newIntervals[i] = newinterval;
              onChange({ ...mode, intervals: newIntervals });
            }} />
          </div>
          <div className="d-inline-block flex-fill p-1">
            <Form.Select value={interval.mode} onChange={e => {
              const newinterval = { ...interval, mode: +e.currentTarget.value };
              const newIntervals = [...mode.intervals];
              newIntervals[i] = newinterval;
              onChange({ ...mode, intervals: newIntervals });
            }}>
              {modes
                .filter((m) => m.type !== "EMPTY")
                .filter(m => m.index != mode.index)
                .map(m => m as BaseMode)
                .map((m, mi) => {
                  return <option key={mi} value={m.index}>{m.name}</option>;
                })}
              <option key={-1} value={-1}>Off</option>;
            </Form.Select>
          </div>
            <div className="d-inline-block w-25 p-1">
              <Button className="action float-end" type="button" variant="danger" disabled={mode.intervals.length < 3} onClick={() => {
                const newIntervals = [...mode.intervals];
                newIntervals.splice(i, 1);
                onChange({ ...mode, intervals: newIntervals });
              }}>
                Delete
              </Button>
            </div>
        </div>);
      })}
      <div className="d-flex justify-content-end">
        <div className="d-inline-block w-25 p-1">
          <Button type="button" variant="info" className="action float-end" onClick={() => {
            const newIntervals = [...mode.intervals];
            newIntervals.push({ seconds: 10, mode: -1 });
            onChange({ ...mode, intervals: newIntervals });
          }}>
            Add
          </Button>
        </div>
      </div>
    </Form.Group>
  </>;
}
