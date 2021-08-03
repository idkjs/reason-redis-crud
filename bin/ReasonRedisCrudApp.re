open Opium;
module Redis = Redis_lwt.Client;

let host = Array.length(Sys.argv) >= 2 ? Sys.argv[1] : "localhost";
let port = 6379;

let conn_promise = Redis.connect({host, port});

let (let.lwt) = Lwt.bind;
let (let.lwt_map) = (v, f) => Lwt.map(f, v);

let not_found = Rock.Response.make(~status=`Not_found, ());

let get_key =
  App.get("/:key", req => {
    let key = Router.param(req, "key");
    let.lwt conn = conn_promise;
    let.lwt_map response = Redis.get(conn, key);
    switch (response) {
    | Some(value) => Response.of_plain_text(key ++ ": " ++ value)
    | None => not_found
    };
  });

let set_key =
  App.put("/:key", req => {
    let key = Router.param(req, "key");
    let.lwt value = req.Request.body |> Body.to_string;
    let.lwt conn = conn_promise;
    let.lwt _success = Redis.set(conn, key, value);
    Response.of_plain_text("SET " ++ key ++ " " ++ value) |> Lwt.return;
  });
let delete_key =
  App.delete("/:key", req => {
    let key = Router.param(req, "key");
    let.lwt _value = req.Request.body |> Body.to_string;
    let.lwt conn = conn_promise;
    let.lwt _success = Redis.del(conn, [key]);
    Response.of_plain_text("DEL " ++ key) |> Lwt.return;
  });

App.empty
|> App.port(3000)
|> get_key
|> set_key
|> delete_key
|> App.run_command;

