import { Configs, WordClockMode } from "../types";

import { Form } from "react-bootstrap";

type ModeWordClockEditProps = {
  mode: WordClockMode;
  configs: Configs;
  onChange: (mode: WordClockMode) => void;
};
export function ModeWordClockEdit({ mode, configs, onChange }: ModeWordClockEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formDefs">
      <Form.Label>Zeitansagen</Form.Label>
      {Object.keys(configs.times).map((timedIndex, i) => {
        const go = configs.times[timedIndex];
        return <Form.Select key={i} value={mode.times[timedIndex]} className="mb-1" onChange={e => {
          const times = { ...mode.times };
          times[timedIndex] = e.currentTarget.value;
          console.log(times);
          onChange({ ...mode, times: times });
        }}>
          {go.map((t, i) => <option key={i} value={t}>{t}</option>)}
        </Form.Select>;
      })}
    </Form.Group>
  </>;
}
