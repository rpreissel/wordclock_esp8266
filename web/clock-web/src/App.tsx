import { Accordion, Container } from "react-bootstrap";

import LiveView from "./LiveView.tsx";
import ModeEdit from "./edit/ModeEdit.tsx";
import ModeThumbnail from "./ModeThumbnail.tsx";
import ModesManage from "./ModesManage.tsx";
import { modeName } from "./types.ts";
import { useModel } from "./model.ts";

function App() {
  const [model] = useModel();
  console.log("render")
  if (!model) {
    return <div>Loading</div>;
  }

  return (
    <Container fluid >
      <Accordion defaultActiveKey={["Modes"]} alwaysOpen>
        <Accordion.Item eventKey="Modes">
          <Accordion.Header>Modes</Accordion.Header>
          <Accordion.Body>

            <div className="d-flex justify-content-center">
              <div style={{ width: "fit-content", margin: "auto" }}>

                <ModeThumbnail key={-1} mode={{ type: "OFF", index: -1 }} colors={model.colors} active={model.current == -1}
                  onClick={index => model.current = index} />
                {model.modes.map((mode) => (

                  <ModeThumbnail key={mode.index} mode={mode} colors={model.colors} active={model.current == mode.index}
                    onClick={index => model.current = index} />

                ))}
              </div>
            </div>
          </Accordion.Body>
        </Accordion.Item>
        {model.current >= 0 && (<>
          <Accordion.Item eventKey="Configure">
            <Accordion.Header>Configure '{modeName(model.currentMode)} / {model.currentMode.type}'</Accordion.Header>
            <Accordion.Body>
              <div className="d-flex justify-content-center">
                <ModeEdit key={model.currentMode.type + model.current} mode={model.currentMode} configs={model} modes={model.modes} onSave={(mode) => model.changeMode(mode)} />
              </div>
            </Accordion.Body>
          </Accordion.Item>
        </>)}
        <Accordion.Item eventKey="View">
          <Accordion.Header>View Clock</Accordion.Header>
          <Accordion.Body>
            <div className="d-flex justify-content-center">
              <LiveView colors={model.colors} />
            </div>
          </Accordion.Body>
        </Accordion.Item>
        <Accordion.Item eventKey="Organize">
          <Accordion.Header>Organize Modes</Accordion.Header>
          <Accordion.Body>
            <div className="d-flex justify-content-center">
              <ModesManage key={model.version} modes={model.modes.map(m => ({index:m.index, type:m.type, name:modeName(m)}))}
              configs={model} onSave={modes => model.changeModes(modes)}/>
            </div>
          </Accordion.Body>
        </Accordion.Item>
      </Accordion>
    </Container>
  );
}

export default App;
