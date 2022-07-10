# rioki's signal library

## Getting Started

rsig provides a signal class that is simple and thread safe. All you need to do 
is create a signal, hock up a function to it and start handling events.

For the start, let's take this contrived example:

    rsig::signal<unsigned int, unsigned int> processing_signal;

    processing_signal.connect([&] (auto done, auto total) {
        auto percent = static_cast<float>(done) / static_cast<float>(total) * 100.0f;
        std::cout << "Handled " << done << " of " << total << " [" << percent << "%]" << std::endl;
    });

    for (auto i = 0u; i < items.size(); i++)
    {
        process_item(items[i]);
        processing_signal.emit(i+1, items.size());
    }

Although contrived, this example shows the basic usage pattern of rsig. You
create a signal, connect any number of observers and emit signals when ever 
necessary.

## Life Time

But life is not so simple, take the following a bit less contrived example. 
You have a mouse class, a bit like so:

    class Mouse
    {
    public:

        rsig::signal<int, int>& get_move_signal()
        {
            return move_signal;
        }

        void update()
        {
            // get x and y from the OS
            move_signal.emit(x, y);
        }

    private:
        rsig::signal<int, int> move_signal;
    };

If you want to observe the mouse motion in a game, you would write a player
controller a bit like so:

    class PlayerController
    {
    public:
        void activate(Mouse& mouse)
        {
            move_con = mouse.get_move_signal().connect([this] (auto x, auto y) {
                control(x, y);
            });
        }

        void deactivate(Mouse& mouse)
        {
            mouse.get_move_signal().disconnect(move_con);
        }

        void control(int x, int y)
        {
            // magic and unicorns
        }

    private:
        rsig::connection move_con;
    };

The connection object is a opaque handle to the handler registration. All you
need to do is save that handle and pass it to disconnect once you are done with 
handling events.

## Thread Safety

The signal class is written with multi-threading in mind. You can connect and
disconnect handlers while emitting events. The signal is protected by a mutex,
the downside is that it may block while signal emission is handled. 

## Caveats

Though shalt not emit signals recursively. For one, the built in mutex will block,
but even if that was not the case you would probably run into weird and undefined
behavior. 

Though shalt not connect or disconnect from a signal handler. Again the mutex
is the party pooper here. Although it is possible to make connect and disconnect
from a hanlder possible, but to do this the handler list needs to be copied on
every signal emission. This is a huge performance hit, which is unreasonable 
for a use case that is a bad idea anyway. 
