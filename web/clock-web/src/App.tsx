import { Accordion, Container } from "react-bootstrap";

import LiveView from "./LiveView.tsx";
import ModeEdit from "./ModeEdit.tsx";
import ModeThumbnail from "./ModeThumbnail.tsx";
import { useModel } from "./model.ts";

function App() {
  const [model] = useModel();
  console.log("render")
  if (!model) {
    return <div>Loading</div>;
  }

  return (
    <Container fluid >
      <Accordion defaultActiveKey={["0"]} alwaysOpen>
        <Accordion.Item eventKey="0">
          <Accordion.Header>Modes</Accordion.Header>
          <Accordion.Body>

            <div className="d-flex justify-content-center">
              <div style={{ width: "fit-content", margin: "auto" }}>

                <ModeThumbnail key={-1} mode={{ type: "OFF", index: -1 }} colors={model.colors} active={model.current==-1}
                onClick={index => model.current = index} />
                {model.modes.map((mode) => (

                  <ModeThumbnail key={mode.index} mode={mode} colors={model.colors} active={model.current==mode.index}
                  onClick={index => model.current = index} />

                ))}
              </div>
            </div>
          </Accordion.Body>
        </Accordion.Item>
        <Accordion.Item eventKey="1">
          <Accordion.Header>Edit Mode</Accordion.Header>
          <Accordion.Body>
            <div className="d-flex justify-content-center">
              <ModeEdit key={model.current} mode={model.modes[model.current]} configs={model} onSave={(mode) => model.changeMode(mode)}/>
            </div>
          </Accordion.Body>
        </Accordion.Item>
        <Accordion.Item eventKey="2">
          <Accordion.Header>Live</Accordion.Header>
          <Accordion.Body>
            <div className="d-flex justify-content-center">
              <LiveView colors={model.colors} />
            </div>
          </Accordion.Body>
        </Accordion.Item>
      </Accordion>
    </Container>
  );
}

export default App;
